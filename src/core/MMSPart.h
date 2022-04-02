#ifndef FREEMMS_MMSPART_H
#define FREEMMS_MMSPART_H

#include <list>
#include "Field.h"

class MMSPart {
private:
    std::list<field> _header;
    char *_data;
    long _dataLen;
public:
    MMSPart(std::list<field> header, char *dat, long len);

    MMSPart(const MMSPart &part);

    MMSPart(MMSPart &&part);

    MMSPart &operator=(const MMSPart &part) noexcept;

    ~MMSPart();

    const std::list<field> &header();

    const char *data();

    long dataLen() const {
        return _dataLen;
    }
};

#endif //FREEMMS_MMSPART_H
