#include "MMSHexData.h"
#include <spdlog/spdlog.h>

using namespace spdlog;

MMSHexData::MMSHexData(std::size_t length, char *data) : _length(length), _data(data) {
    spdlog::info("MMSHexData[{}] construct", (void *) this);
}

MMSHexData::MMSHexData(const MMSHexData &hexData) {
    _data = new char[hexData._length];
    memcpy(_data, hexData._data, hexData._length);
    _length = hexData._length;

    spdlog::info("MMSHexData[{}] copy construct", (void *) this);
}

MMSHexData::MMSHexData(MMSHexData &&hexData) {
    spdlog::info("MMSHexData[{}] move construct form {}", (void *) this, (void *) &hexData);
    _data = hexData._data;
    _length = hexData._length;

    hexData._data = nullptr;
    hexData._length = 0;
}

MMSHexData &MMSHexData::operator=(const MMSHexData &hexData) {
    if (&hexData != this) {
        delete[] _data;
        _data = new char[hexData._length];
        memcpy(_data, hexData._data, hexData._length);
        _length = hexData._length;
    }
    return *this;
}


MMSHexData::~MMSHexData() {
    spdlog::info("~MMSHexData [{}]; data address {}", (void *) this, (void *) _data);
    delete[] _data;
}

unsigned char MMSHexData::getChar(std::size_t position) {
    return *(_data + position);
}



