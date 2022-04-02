#ifndef FREEMMS_MMSPARSERCURSOR_H
#define FREEMMS_MMSPARSERCURSOR_H

#include <cstddef>
#include "MMSHexData.h"

class MMSParserCursor {

public:
    explicit MMSParserCursor(MMSHexData &data);

    MMSParserCursor(MMSHexData &data, size_t offset);

    MMSParserCursor &operator=(const MMSParserCursor &c);

    ~MMSParserCursor();

private:
    size_t _gOffset;
    MMSHexData &_data;

public:
    unsigned char operator*() const {
        return _data.getChar(_gOffset);
    }

    const char *data() const {
        return _data.data() + _gOffset;
    }

    const MMSHexData &hexData() const {
        return _data;
    }

    unsigned char operator[](size_t index) const {
        return at(index);
    }

    unsigned char at(size_t index) const {
        return _data.getChar(_gOffset + index);
    }

    const MMSParserCursor operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    MMSParserCursor &operator++() {
        _gOffset++;
        return *this;
    }

    MMSParserCursor offset(size_t offset) const {
        return {_data, _gOffset + offset};
    }

    size_t offset() const {
        return _gOffset;
    }
};

typedef struct MMSParserCursor cursor;

#endif //FREEMMS_MMSPARSERCURSOR_H
