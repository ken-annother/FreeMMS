#include "MMSHexDataParser.h"
#include <spdlog/spdlog.h>
#include <iconv.h>

#define CONTENT_TYPE "Content-Type"

using namespace std;

/**
 * 读取短整数
 *
 * From wap-230-wsp-20010705-a.pdf  MMSV *readQuotedString();
 * Short-integer = OCTET
 * Integers in range 0-127 shall be encoded as a one
 * octet value with the most significant bit set to one (1xxx xxxx)
 * and with the value in the remaining least significant bits.
 *
 * 编码后首字节的范围在 128-255
 * Short-integer = OCTET 8bit字节,范围必须在 0-127,最高位为1
 *
 * @return 返回解码后的值, 如果不符合规范,返回空
*/
static int readShortInteger(const cursor &c, size_t &len) {
    len = 1;
    auto markV = *c;
    if (markV < 128) {
        spdlog::warn("read short integer error, start {} , end {}", c.offset(), c.offset() + 1);
    }
    return markV & 0x7F;
}

/**
 * 读令牌
 *  @Note: 不符合规范不会自动回退,如果需要回退可手动调用
 *
 * Token-text: Token End-of-string
 * Token = 1*<any CHAR except CTLs or separators>
 * separators = "("(40) | ")"(41) | "<"(60) | ">"(62) | "@"(64)
 *            | ","(44) | ";"(59) | ":"(58) | "\"(92) | <">(34)
 *            | "/"(47) | "["(91) | "]"(93) | "?"(63) | "="(61)
 *            | "{"(123) | "}"(125) | SP(32) | HT(9)
 * CHAR = <any US-ASCII character (octets 0 - 127)>
 * CTL = <any US-ASCII control character (octets 0 - 31) and DEL (127)>
 * SP  = <US-ASCII SP, space (32)>
 * HT = <US-ASCII HT, horizontal-tab (9)>
 * End-of-string = <Octet 0>
 *
 * @return 返回读取的字符串
 */
static std::string readTokenText(cursor c, size_t &len) {
    std::vector<unsigned char> dat;

    char tmp;
    do {
        tmp = *c++;
        dat.push_back(tmp);
    } while (tmp != '\0');

    len = dat.size();
    return {dat.begin(), dat.end() - 1};
}

/**
 * 读字符串
 *
 * Text-string = [Quote] *TEXT End-of-string
 * Quote = <Octet 127>
 * End-of-string = <Octet 0>
 * TEXT = <any OCTET except CTLs, but including LWS> 其实就是32-255 + 换行符 CR 0x0D + LF 0x0A  + SPACE 0x20  + TAB 0x09
 * CTL = <any US-ASCII control character (octets 0 - 31) and DEL (127)> 其实就是 0-31, 删除(DEL 0x7F 127)
 * LWS = [CRLF] 1*( SP | HT ) 换行符 (CR 0x0D 13) 换行键 (LF 0x0A 10)  和一个以上的 空格(SP 0x20 32) 水平制表符TAB(HT 0x09 9)
 * 如果 TEXT 文本的第一个字符在 128-255 范围那么必须在文本之前加入 Quote标记,否则就不需要加入该标记.
 * Quote标记不是文本的一部分
 *
 * @param withChartEncode 是否包含字符集编码, 如果包含则不用校验TEXT
 * @return 返回读取的字符串
 */
static std::string readTextString(cursor c, size_t &len) {
    std::vector<unsigned char> dat;

    char tmp;
    do {
        tmp = *c++;
        dat.push_back(tmp);
    } while (tmp != '\0');

    len = dat.size();

    if (dat[0] == 127 && dat[1] > 127) {
        return {dat.begin() + 1, dat.end() - 1};
    } else {
        return {dat.begin(), dat.end() - 1};
    }
}

/**
 * 读取 Uri
 * Uri-value = Text-string
 * URI value SHOULD be encoded per [RFC2616], but service user MAY use a different format.
 * @return
 */
static std::string readUriValue(cursor c, size_t &len) {
    return readTextString(c, len);
}

/**
 * 读取非负整数
 *
 * From wap-230-wsp-20010705-a.pdf
 * The maximum size of a uintvar is 32 bits.
 * So it will be encoded in no more than 5 octets.
 *
 * 该值不允许大于 32bit
 *
 * @return
 */
static long readUIntVarInteger(cursor c, size_t &len) {
    len = 1;
    auto temp = *c;
    long result = 0;
    while ((temp & 0x80) != 0) {
        result = result << 7;
        result |= temp & 0x7F;
        temp = *++c;
        len++;
    }

    result = result << 7;
    result |= temp & 0x7F;
    return result;
}

/**
 * 读取长度
 *
 * Length = Uintvar-integer
 *
 * @return
 */
static long readLength(const cursor &c, size_t &len) {
    return readUIntVarInteger(c, len);
}


/**
 * 读取短型长度
 *
 * Short-length = <Any Octet 0-30>
 *
 * @return
 */
static int readShortLength(const cursor &c, size_t &len) {
    len = 1;
    return *c;
}

/**
 * 读取值长度
 *
 * 首字节范围在 0-30,31
 *
 * Value-length = Short-length | (Length-quote Length)
 * Length-quote = <Octet 31>

 *
 * @return 返回长度值，如果读取失败, 返回空
 */
static long readValueLength(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV < 31) {
        return readShortLength(c, len);
    }

    if (markV == 31) {
        size_t lenLen;
        long realLen = readLength(c.offset(1), lenLen);
        len = lenLen + 1;
        return realLen;
    } else {
        len = 0;
        return 0;
    }
}

/**
 * 读取长整数
 *
 * 第一个字节必须在0-30的范围内
 *
 * Long-integer = Short-length Multi-octet-integer
 * Multi-octet-integer = 1*30 OCTET 采用大端编码的整数值,必须使用最小位数的编码
 *
 * @return
 */
static long readLongInteger(const cursor &c, size_t &len) {
    size_t shortLength = 0;
    int sl = readShortLength(c, shortLength);

//    const auto *dataBegin = reinterpret_cast<const unsigned char *>(c.begin + shortLength);
    auto cTemp = c.offset(shortLength);
    long result = 0;
    for (int i = 0; i < sl; i++) {
        result |= cTemp[i] << (8 * sl - 8 * i - 8);
    }

    len = shortLength + sl;
    return result;
}

/**
 * 读取整数
 * 编码后的范围在 128-255,  0-30
 *
 * Integer-value = Short-integer | Long-integer
 *
 * @return
 */
static long readIntegerValue(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV < 31) {
        return readLongInteger(c, len);
    } else if (markV > 127) {
        return readShortInteger(c, len);
    } else {
        len = 0;
        spdlog::warn("read integer value error, start {}, end {}", c.offset(), c.offset());
        return 0;
    }
}

/**
 * 读取著名字符集编码
 *
 *  Well-known-charset = Any-charset | Integer-value
 *  Any-charset = <Octet 128>
 *
 * @return 整数类型
 */
static std::string readWellKnowCharset(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV == 128) {
        return "Auto";
    }

    long v = readIntegerValue(c, len);
    return metaDataManager.findCharacterSetByCode(v & 0x7F);
}

/**
 * 读取字符集
 *
 * charset = Well-known-charset|Text-String
 *
 * @return 整数或者字符串类型
 */
static std::string readCharset(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 30 && markV < 128) {
        return readTextString(c, len);
    } else {
        return readWellKnowCharset(metaDataManager, c, len);
    }
}

/**
 * 读取带字符集编码的字符传值
 *
 * Encoded-string-value = Text-string | Value-length Char-set Text-string

 *
 *  From oma-ts-mms-conf-v1_3.pdf  MMS规格一致兼容性文档
 *  From wap-230-wsp-20010705-a.pdf
 *
 *  创建时,字符集应为us-ascii(IANA MIBenum 3) 或 UTF-8(IANA MIBenum 106)[Unicode]
 *  在检索时，应同时支持us-ascii和utf-8
 *
 * 该值字符集编码的值用的是 INAN 的 MIBEnum 值
 * 参见: https://www.iana.org/assignments/character-sets/character-sets.xhtml
 *+
 * @return
 */
static std::string readEncodedStringValue(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 31) {
        return readTextString(c, len);
    }

    size_t vlen;
    long valueLength = readValueLength(c, vlen);

    size_t charsetLen;
    string charset = readCharset(metaDataManager, c.offset(vlen), charsetLen);

    size_t tLen;
    string rawText = readTextString(c.offset((vlen + charsetLen)), tLen);
    len = vlen + charsetLen + tLen;

    if (charsetLen + tLen != valueLength) {
        spdlog::warn("read encode string value error, start {} , end {}", c.offset(), c.offset() + len);
    }

    if (charset.length() > 0 && (charset == "UTF-8" || charset == "Auto")) {
        return rawText;
    }

    iconv_t cd;
    cd = iconv_open("UTF-8", charset.c_str());
    if (cd == nullptr) {
        spdlog::warn("cannot support convert charset {} to UTF-8", charset);
        return rawText;
    }

    char *inChar = const_cast<char *>(rawText.c_str());

    size_t inLen, outLen;
    inLen = strlen(inChar);
    outLen = inLen + 1;
    char outText[outLen];
    char *outChar = outText;
    iconv(cd, &inChar, &inLen, &outChar, &outLen);
    iconv_close(cd);
    return {outText};
}

/**
 *  读取扩展媒体类型
 * 首字节范围在 >32
 *
 * Extension-media = *TEXT End-of-string 用于表示在没有对应 well-know 二进制编码的媒体值
 * @return
 */
static std::string readExtensionMedia(MMSMetaDataManager &metaDataManager, cursor c, size_t &len) {
    std::vector<unsigned char> dat;

    char tmp;
    do {
        tmp = *c++;
        dat.push_back(tmp);
    } while (tmp != '\0');

    len = dat.size();
    return {dat.begin(), dat.end() - 1};
}

/**
 * 读取著名媒体类型
 * Well-known-media = Integer-value
 *
 * @return
 */
static std::string readWellKnownMedia(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    long mediaTypeCode = readIntegerValue(c, len);
    return metaDataManager.findContentTypeByCode(mediaTypeCode & 0x7F);
}


/**
 *
 * 读取引用字符串
 *
 * Quoted-string = <Octet 34> *TEXT End-of-string
 *
 * @return
 */
static string readQuotedString(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV != 34) {
        spdlog::warn("read quoted string error, start {}, end {}", c.offset(), c.offset() + 1);
    }

    cursor ac = c.offset(1);
    std::vector<unsigned char> dat;

    char tmp;
    do {
        tmp = *ac++;
        dat.push_back(tmp);
    } while (tmp != '\0');

    len = dat.size() + 1;

    string result = {dat.begin(), dat.end() - 1};
    return "\"" + result + "\"";
}

/**
 * 读取空值
 * No-value = <Octet 0>
 *
 * @return
 */
static string readNoValue(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV != 0) {
        spdlog::warn("read no value error, start {}, end {}", c.offset(), c.offset() + 1);
    }
    len = 1;
    return "";
}

/**
 * 读取文本值
 *
 *  首字节范围在 0, token, 34
 *
 * Text-value = No-value | Token-text | Quoted-string
 *
 * @return
 */
static string readTextValue(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV == 0) {
        return readNoValue(c, len);
    } else if (markV == 34) {
        return readQuotedString(c, len);
    } else {
        return readTokenText(c, len);
    }
}

/**
 * Untyped-value = Integer-value | Text-value
 * @return
 */
static string readUntypedValue(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 30 && markV < 128) {
        return readTextValue(c, len);
    } else {
        return to_string(readIntegerValue(c, len));
    }
}

/**
 * 读取未标记的参数
 *
 * Untyped-parameter = Token-text Untyped-value

 *
 * @return
 */
static string readUntypedParameter(const cursor &c, size_t &len) {
    size_t ttLen, uvLen;
    string token = readTokenText(c, ttLen);
    string value = readUntypedValue(c.offset(ttLen), uvLen);
    len = ttLen + uvLen;
    if (value.length() == 0) {
        return token;
    } else {
        return token + "=" + value;
    }
}

/**
 * 读取著名参数口令
 *  Well-known-parameter-token = Integer-value
 * @return
 */
static string readWellKnownParameterToken(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    long paramToken = readIntegerValue(c, len);
    return metaDataManager.findParamWellknownByCode(paramToken & 0x7F);
}

/**
 * 读取 Q值
 *
 *  Q-value = 1*2 OCTET
 * ; The encoding is the same as in Uintvar-integer, but with restricted size. When quality factor 0
 * ; and quality factors with one or two decimal digits are encoded, they shall be multiplied by 100
 * ; and incremented by one, so that they encode as a one-octet value in range 1-100,
 * ; ie, 0.1 is encoded as 11 (0x0B) and 0.99 encoded as 100 (0x64). Three decimal quality
 * ; factors shall be multiplied with 1000 and incremented by 100, and the result shall be encoded
 * ; as a one-octet or two-octet uintvar, eg, 0.333 shall be encoded as 0x83 0x31.
 * ; Quality factor 1 is the default value and shall never be sent.
 *
 * @return
 */
static string readQValue(const cursor &c, size_t &len) {
    long v = readUIntVarInteger(c, len);
    return to_string(v);
}


/**
 * 读取版本值
 *
 * Version-value = Short-integer | Text-string
 *  ; The three most significant bits of the Short-integer value are interpreted to encode a major
 *  ; version number in the range 1-7, and the four least significant bits contain a minor version
 *  ; number in the range 0-14. If there is only a major version number, this is encoded by
 *  ; placing the value 15 in the four least significant bits. If the version to be encoded fits these
 *  ; constraints, a Short-integer must be used, otherwise a Text-string shall be used.
 *
 * @return
 */
static string readVersionValue(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 127) {
        long versionCode = readShortInteger(c, len);
        return std::to_string(versionCode / 0x10) + "." + std::to_string(versionCode % 0x10);
    }
    return readTextString(c, len);
}

/**
 * 读取秒级差值
 *
 * Delta-seconds-value = Integer-value
 *
 * @return
 */
static long readDeltaSecondsValue(const cursor &c, size_t &len) {
    return readIntegerValue(c, len);
}


/**
 * 读取受约束的编码
 *
 * Constrained-encoding = Extension-Media | Short-integer 用于没有 well-know二进制的令牌字段值编码,或者 well-know 二进制编码值
 * 在 32-127,128-255 范围内
 *
 * @return
 */
static string readConstrainedEncoding(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 127) {
        auto si = readShortInteger(c, len);
        return metaDataManager.findContentTypeByCode(si & 0x7F);
    } else {
        return readExtensionMedia(metaDataManager, c, len);
    }
}

/**
 *
 * 读取受约束的媒体类型
 *
 * Constrained-media = Constrained-encoding 首字节范围在 32-127 128-255
 *
 * @return
 */
static std::string readConstrainedMedia(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    return readConstrainedEncoding(metaDataManager, c, len);
}

/**
 * 读取日期时间
 *
 * Date-value = Long-integer
 * ; The encoding of dates shall be done in number of seconds from
 * ; 1970-01-01, 00:00:00 GMT.
 *
 * @return
 */
static long readDateValue(const cursor &c, size_t &len) {
    return readLongInteger(c, len);
}

/**
 * 读取标记的参数
 *
 * Typed-parameter = Well-known-parameter-token Typed-value
 * Typed-value = Compact-value | Text-value
 * Compact-value = Integer-value | Date-value | Delta-seconds-value | Q-value | Version-value | Uri-value
 *
 * @return
 */
static string readTypedParameter(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    size_t wkLen, vLen;
    string token = readWellKnownParameterToken(metaDataManager, c, wkLen);

    cursor ac = c.offset(wkLen);
    string v;
    if (token == "Q") {
        v = readQValue(ac, vLen);
    } else if (token == "Charset") {
        v = readWellKnowCharset(metaDataManager, ac, vLen);
    } else if (token == "Level") {
        v = readVersionValue(ac, vLen);
    } else if (token == "Type,1.1" || token == "Size") {
        v = to_string(readIntegerValue(ac, vLen));
    } else if (token == "Name,1.1" || token == "Filename,1.1" || token == "Start,1.2" || token == "Start-Info,1.2"
               || token == "Comment,1.3" || token == "Domain,1.3" || token == "Path,1.3") {
        v = readTextString(ac, vLen);
    } else if (token == "Differences" || token == "Padding" || token == "SEC") {
        v = to_string(readShortInteger(ac, vLen));
    } else if (token == "Type,1.2") {
        v = readConstrainedEncoding(metaDataManager, ac, vLen);
    } else if (token == "Max-Age") {
        v = to_string(readDeltaSecondsValue(ac, vLen));
    } else if (token == "Secure") {
        v = readNoValue(ac, vLen);
    } else if (token == "MAC" || token == "Name,1.4" || token == "Filename,1.4" || token == "Start,1.4" ||
               token == "Start-Info,1.4" || token == "Comment,1.4" || token == "Domain,1.4" || token == "Path,1.4") {
        v = readTextValue(ac, vLen);
    } else if (token == "Creation-Date" || token == "Modification-Date" || token == "Read-Date") {
        v = to_string(readDateValue(ac, vLen));
    } else {
        v = "";
        vLen = 0;
    }

    len = wkLen + vLen;

    if (v.length() == 0) {
        return token;
    } else {
        if (token.find(',') != string::npos) {
            return token.substr(0, token.find_first_of(',')) + "=" + v;
        } else {
            return token + "=" + v;
        }
    }
}

/**
 *
 * 读取参数
 *
 * Parameter = Typed-parameter | Untyped-parameter 首字节范围在128-255,0-30选择 Typed-parameter, 32-127 选择 Untyped-parameter
 *
 * @return
 */
static string readParameter(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 31 && markV < 128) {
        return readUntypedParameter(c, len);
    } else if (markV == 31) {
        spdlog::warn("read parameter error, start {}, end {}", c.offset(), c.offset());
        return "";
    } else {
        return readTypedParameter(metaDataManager, c, len);
    }
}

inline list<string>
readParameters(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len, size_t contentLen) {
    len = 0;
    size_t tempLen;
    list<string> params;
    while (len < contentLen) {
        string param = readParameter(metaDataManager, c.offset(len), tempLen);
        len += tempLen;
        params.push_back(param);
    }
    return params;
}

/**
 *
 * 读取 MMS 媒体类型
 *
 * Media-type = (Well-known-media | Extension-Media) *(Parameter)
 * @return
 */
static std::string readMediaType(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len, size_t contentLen) {
    auto markV = *c;
    string mediaType;
    size_t mediaTypeLen;
    if (markV > 30 && markV < 128) {
        mediaType = readExtensionMedia(metaDataManager, c, mediaTypeLen);
    } else {
        mediaType = readWellKnownMedia(metaDataManager, c, mediaTypeLen);
    }

    size_t paramContentLen = contentLen - mediaTypeLen;
    size_t paramLen;
    list<string> param = readParameters(metaDataManager, c.offset(mediaTypeLen), paramLen, paramContentLen);

    if (!param.empty()) {
        mediaType.append(";");
        for (string &p: param) {
            mediaType.append(p);
            mediaType.append(",");
        }
    }

    return mediaType;
}


/**
 * 读取 MMS 内容编码的通常形式
 *
 * Content-general-form = Value-length Media-type
 *
 * @return
 */
static std::string readContentGeneralForm(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    size_t vl;
    auto vlv = readValueLength(c, vl);
    len = vl + vlv;
    size_t mtLen;
    return readMediaType(metaDataManager, c.offset(vl), mtLen, vlv);
}


/**
 * 读取 MMS content-type
 *
 * From wap-230-wsp-20010705-a.pdf
 *
 * http://mirrors.zju.edu.cn/rfc/rfc2387.html
 *
 * Content-type-value = Constrained-media | Content-general-form
 * 优先考察 Content-general-form比较合理
 *
 * @return
 */
static std::string readContentType(MMSMetaDataManager &metaDataManager, const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 31) {
        return readConstrainedMedia(metaDataManager, c, len);
    } else {
        return readContentGeneralForm(metaDataManager, c, len);
    }
}


MMSInfo MMSHexDataParser::parse() {
    MMSInfo info;
    this->parseHeader(info);

    if (info.hasBody()) {
        this->parseBody(info);
    }
    return info;
}

void MMSHexDataParser::parseHeader(MMSInfo &info) {
    bool endOfHeader = false;
    while (!endOfHeader) {
        auto headerFieldCode = this->mmsHexData.getChar(currentPos);
        string headerField = this->metaDataManager.findFieldNameByCode(headerFieldCode);
        currentPos++;

        field f;
        f.name = {headerField, currentPos - 1, currentPos};
        f.value = parseHeaderFieldByType(headerField);

        spdlog::info("code {}, name is : {}, value is : {} \n",
                     (unsigned char) headerFieldCode,
                     headerField.c_str(),
                     f.value.value.c_str());

        info.addHeaderField(f);

        if (headerField == CONTENT_TYPE) {
            endOfHeader = true;
        }
    }
}

void MMSHexDataParser::parseBody(MMSInfo &info) {
    cursor c = {mmsHexData, currentPos};
    int partNum = *c;
    spdlog::debug("parse body part count is {}", partNum);
    currentPos++;

    size_t len;
    for (int i = partNum; i > 0; i--) {
        info.addPart(parsePart({mmsHexData, currentPos}, len));
        spdlog::info("MMSHexDataParser::parseBody TAG 1");
        currentPos += len;
    }
}

MMSPart MMSHexDataParser::parsePart(const cursor &c, size_t &len) {
    size_t partHeaderLenUsedSize;
    size_t partHeaderLen = readUIntVarInteger(c, partHeaderLenUsedSize);

    cursor ac = c.offset(partHeaderLenUsedSize);
    size_t parDataLenUsedSize;
    long partDataLen = readUIntVarInteger(ac, parDataLenUsedSize);

    list<field> headerFields = parsePartHeaders(
            metaDataManager,
            c.offset((partHeaderLenUsedSize + parDataLenUsedSize)),
            partHeaderLen);

    char *data = new char[partDataLen];
    memcpy(data, c.offset(partHeaderLenUsedSize + parDataLenUsedSize + partHeaderLen).data(), partDataLen);

    len = partHeaderLenUsedSize + parDataLenUsedSize + partHeaderLen + partDataLen;
    return {headerFields, data, partDataLen};
}


MMSV<std::string> MMSHexDataParser::parseHeaderFieldByType(const std::string &fieldName) {
    MMSV<std::string> result = {};
    result.start = currentPos;
    size_t len;
    cursor cur = {this->mmsHexData, currentPos};
    if (fieldName == "Message-Type") {
        result.value = parseHeaderOfXMmsMessageType(cur, len);
    } else if (fieldName == "MMS-Version") {
        result.value = parseHeaderOfXMmsMMSVersion(cur, len);
    } else if (fieldName == "Message-Class") {
        result.value = parseHeaderOfXMmsMessageClass(cur, len);
    } else if (fieldName == "Priority") {
        result.value = parseHeaderOfXMmsPriority(cur, len);
    } else if (fieldName == "Delivery-Report") {
        result.value = parseHeaderOfXMmsDeliveryReport(cur, len);
    } else if (fieldName == "Read-Reply") {
        result.value = parseHeaderOfXMmsReadReply(cur, len);
    } else if (fieldName == "Transaction-Id") {
        result.value = parseHeaderOfXMmsTransactionId(cur, len);
    } else if (fieldName == "Message-ID") {
        result.value = parseHeaderOfMessageId(cur, len);
    } else if (fieldName == "Date") {
        result.value = parseHeaderOfDate(cur, len);
    } else if (fieldName == "To") {
        result.value = parseHeaderOfTo(cur, len);
    } else if (fieldName == "From") {
        result.value = parseHeaderOfFrom(cur, len);
    } else if (fieldName == "Subject") {
        result.value = parseHeaderOfSubject(cur, len);
    } else if (fieldName == "Cc") {
        result.value = parseHeaderOfCc(cur, len);
    } else if (fieldName == "Content-Type") {
        result.value = parseHeaderOfContentType(cur, len);
    } else if (fieldName == "Content-Location") {
        result.value = parseHeaderOfXMmsContentLocation(cur, len);
    } else if (fieldName == "Expiry") {
        result.value = parseHeaderOfXMmsMMSExpiry(cur, len);
    } else if (fieldName == "Message-Size") {
        result.value = parseHeaderOfXMmsMMSMessageSize(cur, len);
    } else {
        len = 1;
    }

    result.end = currentPos + len;
    currentPos += len;
    return result;
}


/**
 * 读取 Message-Type 字段
 *
 * Message-type-value = m-send-req | m-send-conf | m-notification-ind |
 * m-notifyresp-ind | m-retrieve-conf | m-acknowledge-ind | m-delivery-ind
 *
 * m-send-req = <Octet 128>
 * m-send-conf = <Octet 129>
 * m-notification-ind = <Octet 130>
 * m-notifyresp-ind = <Octet 131>
 * m-retrieve-conf = <Octet 132>
 * m-acknowledge-ind = <Octet 133>
 * m-delivery-ind = <Octet 134>
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsMessageType(const cursor &c, size_t &len) {
    len = 1;
    return metaDataManager.findMessageTypeNameByCode(*c);
}

/**
 * MMS版本
 *
 * MMS-version-value = Short-integer
 *
 * The three most significant bits of the Short-integer are interpreted to encode a major version number in the range 1-7,
 * and the four least significant bits contain a minor version number in the range 0-14. If there is only a major version
 * number, this is encoded by placing the value 15 in the four least significant bits [WAPWSP].
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsMMSVersion(const cursor &c, size_t &len) {
    int number = readShortInteger(c, len);
    return std::to_string(number / 0x10) + "." + std::to_string(number % 0x10);
}

/**
 * MMS类型
 *
 * Message-class-value = Class-identifier | Token-text
 * Class-identifier = Personal | Advertisement | Informational | Auto
 *
 * Personal = <Octet 128>
 * Advertisement = <Octet 129>
 * Informational = <Octet 130>
 * Auto = <Octet 131>
 * The token-text is an extension method to the message class.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsMessageClass(const cursor &c, size_t &len) {
    auto markV = *c;
    if (markV > 127) {
        len = 1;
        return metaDataManager.findMessageClassByCode(markV);
    } else {
        return readTokenText(c, len);
    }
}

/**
 * 读取 MMS 优先级
 *
 * Priority-value = Low | Normal | High
 * Low = <Octet 128>
 * Normal = <Octet 129>
 * High = <Octet 130>
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsPriority(const cursor &c, size_t &len) {
    len = 1;
    return metaDataManager.findPriorityByCode(*c);
}

/**
 * 读取 MMS DR 设置
 *
 * Delivery-report-value = Yes | No
 * Yes = <Octet 128>
 * No = <Octet 129>
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsDeliveryReport(const cursor &c, size_t &len) {
    len = 1;
    return metaDataManager.findDeliveryReportByCode(*c);
}

/**
 * 读取 MMS 阅读回执 设置
 *
 * Read-reply-value = Yes | No
 * Yes = <Octet 128>
 * No = <Octet 129>
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsReadReply(const cursor &c, size_t &len) {
    len = 1;
    return metaDataManager.findReadReplyByCode(*c);
}

/**
 * 读取 MMS 交易流水编号
 *
 * Transaction-id-value = Text-string
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsTransactionId(const cursor &c, size_t &len) {
    return readTextString(c, len);
}

/**
 * 读取 MMS 流水编号
 *
 * Message-ID-value = Text-string
 * Encoded as in email address as per [RFC822]. The characters "<" and ">" are not included.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfMessageId(const cursor &c, size_t &len) {
    return readTextString(c, len);
}

/**
 * 读取 MMS 秒级时间戳
 *
 * Date-value = Long-integer
 * In seconds from 1970-01-01, 00:00:00 GMT.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfDate(const cursor &c, size_t &len) {
    long data = readDateValue(c, len);
    return to_string(data);
}

/**
 * 读取 MMS 接收方地址
 *
 *  To-value = Encoded-string-value
 *  See Chapter 8 for addressing model.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfTo(const cursor &c, size_t &len) {
    return readEncodedStringValue(metaDataManager, c, len);
}

/**
 * 读取 MMS 发送方地址
 *
 * From-value = Value-length (Address-present-token Encoded-string-value | Insert-address-token )
 * Address-present-token = <Octet 128>
 * Insert-address-token = <Octet 129>
 * See Chapter 8 for addressing model.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfFrom(const cursor &c, size_t &len) {
    size_t vLen;
    long vl = readValueLength(c, vLen);
    cursor ac = c.offset(ptrdiff_t(vLen));
    auto markV = *ac;
    if (markV == 129) {
        len = vLen + 1;
        return "[Placeholder]";
    } else if (markV == 128) {
        size_t enLen;
        string result = readEncodedStringValue(metaDataManager, ac.offset(1), enLen);
        len = vLen + 1 + enLen;
        return result;
    } else {
        len = vLen + vl;
        return "";
    }
}

/**
 * 读取 MMS 抄送地址
 *
 * Cc-value = Encoded-string-value
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfCc(const cursor &c, size_t &len) {
    return readEncodedStringValue(metaDataManager, c, len);
}

/**
 *
 * 读取 MMS 主题
 *
 * Subject-value = Encoded-string-value
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfSubject(const cursor &c, size_t &len) {
    return readEncodedStringValue(metaDataManager, c, len);
}


std::string MMSHexDataParser::parseHeaderOfContentType(const cursor &c, size_t &len) {
    return readContentType(metaDataManager, c, len);
}

/**
 * 读取 MMS URL
 * Content-location-value = Uri-value
 * Uri-value = Text-string
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsContentLocation(const cursor &c, size_t &len) {
    return readUriValue(c, len);
}

/**
 * 读取 MMS 有效期
 *
 * Expiry-value = Value-length (Absolute-token Date-value | Relative-token Delta-seconds-value)
 * Absolute-token = <Octet 128>
 * Relative-token = <Octet 129>
 *
 * Length of time the message will be available. The field has only one format, interval.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsMMSExpiry(const cursor &c, size_t &len) {
    size_t vl;
    long vll = readValueLength(c, vl);

    cursor ac = c.offset(vl);
    auto markV = *ac;
    if (markV == 128) {
        ac = c.offset(vl + 1);
        size_t dateLen;
        long timestamp = readDateValue(ac, dateLen);
        len = vl + 1 + dateLen;
        return to_string(timestamp);
    } else if (markV == 129) {
        ac = c.offset((vl + 1));
        long delta = readDeltaSecondsValue(c, len);
        return string("+") + to_string(delta);
    } else {
        len = vl;
        return "0";
    }
}

/**
 * 读取 MMS 大小
 *  Message-size-value = Long-integer
 * 仅仅是参考值
 * Full size of message in octets. The value of this header
 * field could be based on approximate calculation,
 * therefore it SHOULD NOT be used as a reason to reject
 * the MM.
 *
 * @return
 */
std::string MMSHexDataParser::parseHeaderOfXMmsMMSMessageSize(const cursor &c, size_t &len) {
    return to_string(readLongInteger(c, len));
}

std::list<field>
MMSHexDataParser::parsePartHeaders(MMSMetaDataManager &mmsMetaDataManager, const cursor &c, const size_t &contentLen) {
    list<field> fields;

    field f;
    f.name = {"Content-Type", c.offset(), c.offset()};
    size_t contentTypeLen;
    string contentType = readContentType(metaDataManager, c, contentTypeLen);
    f.value = {contentType, c.offset(), c.offset() + contentTypeLen};
    fields.push_back(f);

    size_t usedLen = contentTypeLen;
    while ((contentLen - usedLen) > 0) {
        field tf;

        size_t siLen;
        long fieldParmaCode = readShortInteger(c.offset(usedLen), siLen);
        usedLen += siLen;

        string fieldName = metaDataManager.findParamFieldByCode(fieldParmaCode);
        tf.name = {fieldName, c.offset() + usedLen, c.offset() + usedLen + siLen};

        size_t vLen;
        string value;
        if (fieldName == "Content-ID") {
            value = readQuotedString(c.offset(usedLen), vLen);
        } else if (fieldName == "Content-Location") {
            value = readTextString(c.offset(usedLen), vLen);
        } else {
            vLen = 0;
            value = "";
        }

        tf.value = {value, c.offset() + usedLen, c.offset() + usedLen + vLen};
        usedLen += vLen;
        fields.push_back(tf);
    }


    return fields;
}



