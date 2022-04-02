#include "MMSParserCursor.h"

MMSParserCursor::MMSParserCursor(MMSHexData &data) : _data(data), _gOffset(0) {

}

MMSParserCursor::MMSParserCursor(MMSHexData &data, size_t offset) : _data(data), _gOffset(offset) {

}

MMSParserCursor &MMSParserCursor::operator=(const MMSParserCursor &c) {
    if (this != &c) {
        this->_data = c.hexData();
        this->_gOffset = c.offset();
        return *this;
    }
    return *this;
}

MMSParserCursor::~MMSParserCursor() = default;


