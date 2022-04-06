#include "MMSHexDataGenerator.h"

#include <utility>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

typedef MMSInfo *pInfo;
typedef pInfo pMMSInfo;
using namespace std;

static std::istream &safe_get_line(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return is;
            case '\r':
                if (sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                is.setstate(std::ios::eofbit);       //
                if (t.empty())                        // <== change here
                    is.setstate(std::ios::failbit);  //
                return is;
            default:
                t += (char) c;
        }
    }
}

static vector<string> split(const string& s, const string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}

static vector<string> split(const string& s, const string& delimiter, int max) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos && max > 1) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
        max--;
    }
    res.push_back(s.substr(pos_start));
    return res;
}

MMSHexDataGenerator::MMSHexDataGenerator(MMSMetaDataManager &metaDataManager,
                                         std::string mmsPlain) : _metaDataManager(metaDataManager),
                                                                 _mmsPlain(std::move(mmsPlain)) {
}


mms_hex_data MMSHexDataGenerator::parse() {
    auto info = readFromPlainFile();
    return nullptr;
}


inline static void readHeader(MMSInfo &info, ifstream &steam) {
    string s;
    while (safe_get_line(steam, s)) {
        if (s.empty()) {
            break;
        }

        auto arr = split(s, ": ", 2);
        if (arr.size() == 2) {
            cout << ">>>: " << arr[0] << "|||" << arr[1] << endl;
            field f;
            f.name = {arr[0], 0, 0};
            f.value = {arr[1], 0, 0};
            info.addHeaderField(f);
        }
    }
}

inline static void readBody(MMSInfo &info, ifstream &steam) {
    string s;
    while (getline(steam, s)) {
        auto arr = split(s, ": ", 2);
        if (arr.size() == 2) {
            cout << ">>>: " << arr[0] << "|||" << arr[1] << endl;
        }
    }
}

MMSInfo *MMSHexDataGenerator::readFromPlainFile() {
    ifstream ifs(_mmsPlain);
    if (!ifs.is_open()) {
        spdlog::error("cannot open file {}", _mmsPlain);
        return nullptr;
    }

    auto info = new MMSInfo();
    readHeader(*info, ifs);
    if (info->hasBody()) {
        readBody(*info, ifs);
    }
    return info;
}

