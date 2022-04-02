#ifndef FREEMMS_MMSHEXDATA_H
#define FREEMMS_MMSHEXDATA_H

#include <limits>

class MMSHexData {
public:
    MMSHexData(std::size_t length, char *data);

    MMSHexData(const MMSHexData &hexData);

    MMSHexData(MMSHexData &&hexData);

    MMSHexData &operator=(const MMSHexData &hexData);

    ~MMSHexData();

public:
    unsigned char getChar(std::size_t position);

    std::size_t length() const {
        return _length;
    }

    const char *data() const {
        return _data;
    }

private:
    std::size_t _length;
    char *_data;
};


#endif //FREEMMS_MMSHEXDATA_H
