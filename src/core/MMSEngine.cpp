#include "MMSEngine.h"
#include <fstream>
#include <utility>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include "spdlog/spdlog.h"
#include "MMSHexDataParser.h"
#include "MMSInfo.h"

using namespace std;
using namespace boost;

inline static MMSInfo *convertHexFile(MMSMetaDataManager &metaDataManager, const std::string &mmsHexFilePath) {
    ifstream mmsHexFile(mmsHexFilePath);
    if (!mmsHexFile.is_open()) {
        spdlog::error("file not exist {}", mmsHexFilePath);
        return nullptr;
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

    MMSHexDataParser hexDataParser = MMSHexDataParser(metaDataManager, mmsHexData);
    auto mmsInfo = new MMSInfo(hexDataParser.parse());
    delete[] buffer;
    return mmsInfo;
}

MMSEngine::MMSEngine() {
    this->initMMSMetaDataManager({});
}

MMSEngine::MMSEngine(std::string configDir) {
    this->initMMSMetaDataManager(std::move(configDir));
}

MMSEngine::~MMSEngine() {
    delete metaDataManager;
}


void MMSEngine::initMMSMetaDataManager(std::string configDir) {
    if (configDir.length() == 0) {
        configDir = "metadata";
    }

    if (!filesystem::exists(configDir)) {
        spdlog::error("config dir {} not exist", configDir);
        throw runtime_error("config dir " + configDir + " not exist");
    }

    this->metaDataManager = new MMSMetaDataManager(configDir);
}


std::string MMSEngine::convert2Plain(const std::string &mmsHexFilePath, bool withBinaryBody) {
    MMSInfo *mmsInfo = convertHexFile(*metaDataManager, mmsHexFilePath);
    string data = mmsInfo->toPlain(withBinaryBody);
    delete mmsInfo;
    return data;
}

std::string MMSEngine::convert2Plain(const string &mmsHexFilePath) {
    return convert2Plain(mmsHexFilePath, false);
}

void MMSEngine::convert2PlainFile(const string &mmsHexFilePath, const string &outFile, bool withBinaryBody) {
    string result = convert2Plain(mmsHexFilePath, withBinaryBody);
    ofstream out(outFile);
    if (!out.is_open()) {
        spdlog::error("can not write file {}", outFile);
        return;
    }

    out << result;
    out.flush();
    out.close();
}

void MMSEngine::convert2PlainFile(const string &mmsHexFilePath, const string &outFile) {
    convert2PlainFile(mmsHexFilePath, outFile, false);
}

inline static std::string getPartFileName(const std::list<field> &headInfo) {
    auto it = std::find_if(headInfo.begin(), headInfo.end(), [](const field &f) -> bool {
        return f.name.value == "Content-Location";
    });

    if (it != headInfo.end()) {
        return it->value.value;
    }
    return "";
}

void MMSEngine::convert2PlainDirectory(const string &mmsHexFilePath, const string &outDir) {
    MMSInfo *mmsInfo = convertHexFile(*metaDataManager, mmsHexFilePath);
    string data = mmsInfo->toPlain(false);

    if (!filesystem::exists(outDir)) {
        filesystem::remove(outDir);
        filesystem::create_directory(outDir);
    }

    ofstream fMain(outDir + "/manifest.txt");
    if (!fMain.is_open()) {
        spdlog::error("can not write file {}", outDir + "/manifest.txt");
        return;
    }

    fMain << data;
    fMain.flush();
    fMain.close();

    for (auto &part: *mmsInfo->body()) {
        string fileName = getPartFileName(part->header());
        if (!fileName.empty()) {
            string ft = outDir;
            ft.append("/").append(fileName);
            ofstream fP(ft);
            fP.write(part->data(), part->dataLen());
            fP.flush();
            fP.close();
        }
    }

    delete mmsInfo;
}

MMSHexData *MMSEngine::convert2mmsHex(const std::string &mmsPlain) {
    return nullptr;
}










