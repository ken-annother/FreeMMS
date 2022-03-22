#ifndef FREEMMS_MMSPARSERCURSOR_H
#define FREEMMS_MMSPARSERCURSOR_H

#include <cstddef>

typedef struct MMSParserCursor {
    const char *begin;
    size_t gOffset;

    unsigned char operator*() const {
        return *begin;
    }

    MMSParserCursor offset(ptrdiff_t offset) const {
        return {begin + offset, gOffset + offset};
    }
} cursor;

#endif //FREEMMS_MMSPARSERCURSOR_H
