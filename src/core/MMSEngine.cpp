#include "MMSEngine.h"
#include <fstream>
#include <utility>
#include <memory>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include "MMSHexDataParser.h"
#include "MMSHexDataGenerator.h"
#include "MMSInfo.h"

using namespace std;
using namespace boost;

inline static mms_info
convertHexFile(MMSMetaDataManager &metaDataManager, const std::string &mmsHexFilePath) {
    ifstream mmsHexFile(mmsHexFilePath);
    if (!mmsHexFile.is_open()) {
        spdlog::error("file not exist {}", mmsHexFilePath);
        return nullptr;
    }

    mmsHexFile.seekg(0, ios::end);
    size_t len = mmsHexFile.tellg();
    char *buffer = new char[len];
    mmsHexFile.seekg(0, ios::beg);
    mmsHexFile.read(buffer, static_cast<streamsize>(len));
    mmsHexFile.close();

    MMSHexDataParser hexDataParser = MMSHexDataParser(metaDataManager, {len, buffer});

    return make_shared<MMSInfo>(hexDataParser.parse());
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
    auto mmsInfo = convertHexFile(*metaDataManager, mmsHexFilePath);
    string data = mmsInfo->toPlain(withBinaryBody);
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
    auto mmsInfo = convertHexFile(*metaDataManager, mmsHexFilePath);
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
        string fileName = getPartFileName(part.header());
        if (!fileName.empty()) {
            string ft = outDir;
            ft.append("/").append(fileName);
            ofstream fP(ft);
            fP.write(part.data(), part.dataLen());
            fP.flush();
            fP.close();
        }
    }
}

void MMSEngine::convert2mmsHex(const std::string &mmsPlain, const std::string &outputHexPath) {
    spdlog::info("convert2mmsHex outputHexPath: {}", outputHexPath);
    auto result = convert2mmsHex(mmsPlain);
    if (result != nullptr) {
        ofstream outF(outputHexPath);
        if (!outF.is_open()) {
            spdlog::error("cannot write hex file {}", outputHexPath);
            return;
        }

        outF.write(result->data(), static_cast<streamsize>(result->length()));
        outF.close();
    }
}

mms_hex_data MMSEngine::convert2mmsHex(const string &mmsPlain) {
    spdlog::info("convert2mmsHex mmsPlain: {}", mmsPlain);
    ifstream mmsPlainFile(mmsPlain);
    if (!mmsPlainFile.is_open()) {
        spdlog::error("file not exist {}", mmsPlain);
        return nullptr;
    }

    MMSHexDataGenerator dataGenerator(*metaDataManager, mmsPlain);
    return dataGenerator.parse();
}










