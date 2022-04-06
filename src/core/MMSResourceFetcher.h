#ifndef FREEMMS_MMSRESOURCEFETCHER_H
#define FREEMMS_MMSRESOURCEFETCHER_H

#include <string>

class MMSResourceFetcher {

public:
    void fetch(const std::string& url, char **dat, size_t &len);
};


#endif //FREEMMS_MMSRESOURCEFETCHER_H
