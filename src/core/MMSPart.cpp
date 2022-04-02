#include "MMSPart.h"
#include <spdlog/spdlog.h>

#include <utility>

using namespace std;
using namespace spdlog;

MMSPart::MMSPart(std::list<field> header, char *dat, long len) : _data(dat), _dataLen(len), _header(std::move(header)) {
    spdlog::info("MMSPart[{}]  construct ", (void *) this);
}

MMSPart::MMSPart(MMSPart &&part) {
    spdlog::info("MMSPart[{}] move construct from {}", (void *) this, (void *) &part);
    _data = part._data;
    _dataLen = part._dataLen;
    _header = part._header;

    part._data = nullptr;
    part._dataLen = 0;
}


MMSPart::MMSPart(const MMSPart &part) {
    spdlog::info("MMSPart[{}] copy construct from {}", (void *) this, (void *) &part);

    _data = new char[part._dataLen];
    memcpy(_data, part._data, part._dataLen);

    _dataLen = part._dataLen;
    _header = part._header;
}

MMSPart &MMSPart::operator=(const MMSPart &part) noexcept {
    spdlog::info("MMSPart[{}] assign construct from {}", (void *) this, (void *) &part);

    if (&part != this) {
        delete[] _data;
        _data = new char[part._dataLen];
        memcpy(_data, part._data, part._dataLen);
        _dataLen = part._dataLen;
        _header = part._header;
    }
    return *this;
}


MMSPart::~MMSPart() {
    spdlog::info(":~MMSPart[{}] {}", (void *) this, (void *) _data);
    delete[] _data;
}

const std::list<field> &MMSPart::header() {
    return _header;
}

const char *MMSPart::data() {
    return _data;
}