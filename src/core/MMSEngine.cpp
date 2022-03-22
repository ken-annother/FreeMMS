#include "MMSEngine.h"
#include <fstream>
#include <utility>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include "spdlog/spdlog.h"
#include "MMSHexDataParser.h"
#include "MMSInfo.h"

using namespace std;

MMSEngine::MMSEngine() {
    this->initMMSMetaDataManager({});
}

MMSEngine::MMSEngine(std::string configDir) {
    this->initMMSMetaDataManager(std::move(configDir));
}

void MMSEngine::initMMSMetaDataManager(std::string configDir) {
    if (configDir.length() == 0) {
        configDir = "metadata";
    }

    if (!boost::filesystem::exists(configDir)) {
        spdlog::error("config dir {} not exist", configDir);
        throw runtime_error("config dir " + configDir + " not exist");
    }

    this->metaDataManager = new MMSMetaDataManager(configDir);
}

std::string MMSEngine::convert2Plain(const std::string &mmsHexFilePath) {
    ifstream mmsHexFile(mmsHexFilePath);
    if (!mmsHexFile.is_open()) {
        spdlog::error("file not exist {}", mmsHexFilePath);
        return "";
    }

    mmsHexFile.seekg(0, ios::end);
    streamsize len = mmsHexFile.tellg();
    char *buffer = new char[len];
    mmsHexFile.seekg(0, ios::beg);
    mmsHexFile.read(buffer, len);
    mmsHexFile.close();

    MMSHexData mmsHexData = MMSHexData();
    mmsHexData.length = len;
    mmsHexData.data = buffer;

    MMSHexDataParser hexDataParser = MMSHexDataParser(*this->metaDataManager, mmsHexData);
    MMSInfo mmsInfo = hexDataParser.parse();
    delete[] buffer;

    return mmsInfo.toPlain();
}

MMSHexData *MMSEngine::convert2mmsHex(const std::string &mmsPlain) {
    return nullptr;
}


