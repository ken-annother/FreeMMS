#include "MMSPart.h"
#include <spdlog/spdlog.h>

#include <utility>

using namespace std;
using namespace spdlog;

MMSPart::MMSPart() : _data(nullptr), _dataLen(0) {
}

MMSPart::~MMSPart() {
    delete[] _data;
}

void MMSPart::assignData(char *dat, long len) {
    this->_data = dat;
    this->_dataLen = len;
}

void MMSPart::assignFields(std::list<field> fields) {
    this->_header = std::move(fields);
}

MMSPart::MMSPart(const MMSPart &part) {
    char *d = static_cast<char *>(malloc(sizeof(char) * part._dataLen));
    memcpy(d, part._data, sizeof(char) * part._dataLen);
    this->_data = part._data;
    this->_dataLen = part._dataLen;
    this->_header = part._header;
}

MMSPart::MMSPart(MMSPart &&part) {
    this->_data = part._data;
    this->_dataLen = part._dataLen;
    this->_header = part._header;

    part._data = nullptr;
    part._dataLen = 0;
}

const std::list<field> &MMSPart::header() {
    return _header;
}

const char *MMSPart::data() {
    return _data;
}
