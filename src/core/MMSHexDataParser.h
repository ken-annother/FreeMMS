#ifndef FREEMMS_MMSHEXDATAPARSER_H
#define FREEMMS_MMSHEXDATAPARSER_H

#include <MMSHexData.h>
#include "MMSMetaDataManager.h"
#include "MMSParserCursor.h"
#include "MMSInfo.h"

class MMSHexDataParser {
private:
    MMSMetaDataManager &metaDataManager;
    MMSHexData &mmsHexData;
    size_t currentPos;

    void parseHeader(MMSInfo &info);

    void parseBody(MMSInfo &info);

    std::string parseHeaderOfXMmsMessageType(cursor c, size_t &len);

    static std::string parseHeaderOfXMmsMMSVersion(cursor c, size_t &len);

    std::string parseHeaderOfXMmsMessageClass(cursor c, size_t &len);

    std::string parseHeaderOfXMmsPriority(cursor c, size_t &len);

    std::string parseHeaderOfXMmsDeliveryReport(cursor c, size_t &len);

    std::string parseHeaderOfXMmsReadReply(cursor c, size_t &len);

    static std::string parseHeaderOfXMmsTransactionId(cursor c, size_t &len);

    static std::string parseHeaderOfMessageId(cursor c, size_t &len);

    static std::string parseHeaderOfDate(cursor c, size_t &len);

    std::string parseHeaderOfTo(cursor c, size_t &len);

    std::string parseHeaderOfFrom(cursor c, size_t &len);

    std::string parseHeaderOfSubject(cursor c, size_t &len);

    std::string parseHeaderOfCc(cursor c, size_t &len);

    std::string parseHeaderOfContentType(cursor c, size_t &len);

    static std::string parseHeaderOfXMmsContentLocation(cursor c, size_t &len);

    static std::string parseHeaderOfXMmsMMSExpiry(cursor c, size_t &len);

    static std::string parseHeaderOfXMmsMMSMessageSize(cursor c, size_t &len);

    MMSPart* parsePart(cursor c, size_t &len);

    std::list<field> parsePartHeaders(MMSMetaDataManager &mmsMetaDataManager, cursor c, const size_t &contentLen);

public:
    MMSHexDataParser(MMSMetaDataManager &metaDataManager, MMSHexData &mmsHexData) : metaDataManager(metaDataManager),
                                                                                    mmsHexData(mmsHexData),
                                                                                    currentPos(0) {}

    MMSInfo parse();

    MMSV<std::string> parseHeaderFieldByType(const std::string &basicString);
};


#endif //FREEMMS_MMSHEXDATAPARSER_H
