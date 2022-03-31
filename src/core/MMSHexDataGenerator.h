#ifndef FREEMMS_MMSHEXDATAGENERATOR_H
#define FREEMMS_MMSHEXDATAGENERATOR_H

#include <string>
#include "MMSHexData.h"
#include "MMSMetaDataManager.h"
#include "MMSInfo.h"

class MMSHexDataGenerator {
public:
    MMSHexDataGenerator(MMSMetaDataManager &metaDataManager, std::string mmsPlain);

private:
    MMSMetaDataManager &_metaDataManager;
    std::string _mmsPlain;
public:
    MMSHexData *parse();

private:
    MMSInfo* readFromPlainFile();
};


#endif //FREEMMS_MMSHEXDATAGENERATOR_H
