#include "MMSHexData.h"
#include <spdlog/spdlog.h>

using namespace spdlog;

MMSHexData::MMSHexData(std::size_t length, char *data) : _length(length), _data(data) {

}

MMSHexData::~MMSHexData() {
    spdlog::info("~MMSHexData; data address {}", (void *) _data);
    delete[] _data;
}

unsigned char MMSHexData::getChar(std::size_t position) {
    return *(_data + position);
}
