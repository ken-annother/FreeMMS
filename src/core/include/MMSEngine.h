#ifndef FREEMMS_MMSENGINE_H
#define FREEMMS_MMSENGINE_H

#include <string>
#include "MMSHexData.h"
#include "MMSMetaDataManager.h"

class MMSEngine {
private:
    MMSMetaDataManager* metaDataManager;
    void initMMSMetaDataManager(std::string configDir);

public:
    MMSEngine();
    ~MMSEngine();
    explicit MMSEngine(std::string configDir);

    std::string convert2Plain(const std::string &mmsHexFilePath);
    std::string convert2Plain(const std::string &mmsHexFilePath, bool withBinaryBody);
    void convert2PlainFile(const std::string &mmsHexFilePath, const std::string &outFile);
    void convert2PlainFile(const std::string &mmsHexFilePath, const std::string &outFile, bool withBinaryBody);
    void convert2PlainDirectory(const std::string &mmsHexFilePath, const std::string &outDir);

    void convert2mmsHex(const std::string &mmsPlain, const std::string &outputHexPath);
    mms_hex_data convert2mmsHex(const std::string &mmsPlain);

};


#endif //FREEMMS_MMSENGINE_H
