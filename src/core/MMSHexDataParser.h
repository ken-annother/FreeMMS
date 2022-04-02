#ifndef FREEMMS_MMSHEXDATAPARSER_H
#define FREEMMS_MMSHEXDATAPARSER_H

#include <MMSHexData.h>

#include <utility>
#include "MMSMetaDataManager.h"
#include "MMSParserCursor.h"
#include "MMSInfo.h"

class MMSHexDataParser {
public:
    MMSHexDataParser(MMSMetaDataManager &metaDataManager, MMSHexData mmsHexData) : metaDataManager(metaDataManager),
                                                                                    mmsHexData(std::move(mmsHexData)),
                                                                                    currentPos(0) {}

private:
    void parseHeader(MMSInfo &info);

    void parseBody(MMSInfo &info);

    std::string parseHeaderOfXMmsMessageType(const cursor &c, size_t &len);

    static std::string parseHeaderOfXMmsMMSVersion(const cursor &c, size_t &len);

    std::string parseHeaderOfXMmsMessageClass(const cursor &c, size_t &len);

    std::string parseHeaderOfXMmsPriority(const cursor &c, size_t &len);

    std::string parseHeaderOfXMmsDeliveryReport(const cursor &c, size_t &len);

    std::string parseHeaderOfXMmsReadReply(const cursor &c, size_t &len);

    static std::string parseHeaderOfXMmsTransactionId(const cursor &c, size_t &len);

    static std::string parseHeaderOfMessageId(const cursor &c, size_t &len);

    static std::string parseHeaderOfDate(const cursor &c, size_t &len);

    std::string parseHeaderOfTo(const cursor &c, size_t &len);

    std::string parseHeaderOfFrom(const cursor &c, size_t &len);

    std::string parseHeaderOfSubject(const cursor &c, size_t &len);

    std::string parseHeaderOfCc(const cursor &c, size_t &len);

    std::string parseHeaderOfContentType(const cursor &c, size_t &len);

    static std::string parseHeaderOfXMmsContentLocation(const cursor &c, size_t &len);

    static std::string parseHeaderOfXMmsMMSExpiry(const cursor &c, size_t &len);

    static std::string parseHeaderOfXMmsMMSMessageSize(const cursor &c, size_t &len);

    MMSPart parsePart(const cursor &c, size_t &len);

    std::list<field>
    parsePartHeaders(MMSMetaDataManager &mmsMetaDataManager, const cursor &c, const size_t &contentLen);

public:
    MMSInfo parse();

    MMSV<std::string> parseHeaderFieldByType(const std::string &basicString);

private:
    MMSMetaDataManager &metaDataManager;
    MMSHexData mmsHexData;
    size_t currentPos;
};


#endif //FREEMMS_MMSHEXDATAPARSER_H
