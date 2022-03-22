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
    MMSPart();

    MMSPart(const MMSPart &part);

    MMSPart(MMSPart &&part);

    ~MMSPart();

    const std::list<field> &header();

    const char *data();

    long dataLen() const {
        return _dataLen;
    }

    void assignData(char *data, long len);

    void assignFields(std::list<field> fields);
};

#endif //FREEMMS_MMSPART_H
