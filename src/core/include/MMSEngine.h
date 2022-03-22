#ifndef FREEMMS_MMSENGINE_H
#define FREEMMS_MMSENGINE_H

#include <string>
#include "MMSHexData.h"
#include "../MMSMetaDataManager.h"

class MMSEngine {
private:
    MMSMetaDataManager* metaDataManager;
    void initMMSMetaDataManager(std::string configDir);

public:
    MMSEngine();
    explicit MMSEngine(std::string configDir);

    std::string convert2Plain(const std::string &mmsHexFilePath);

    MMSHexData *convert2mmsHex(const std::string &mmsPlain);

};


#endif //FREEMMS_MMSENGINE_H
