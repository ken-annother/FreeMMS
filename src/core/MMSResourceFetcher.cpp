#include "MMSResourceFetcher.h"
#include <fstream>

using namespace std;

void MMSResourceFetcher::fetch(const std::string &url, char **dat, size_t &len) {
    std::ifstream ifs(url);
    if (ifs.is_open()) {
        ifs.seekg(0, ios::end);
        len = ifs.tellg();
        *dat = new char[len];
        ifs.seekg(0, ios::beg);
        ifs.read(*dat, static_cast<streamsize>(len));
        ifs.close();
    }
}
