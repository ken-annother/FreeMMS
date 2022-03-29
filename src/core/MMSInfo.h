#ifndef FREEMMS_MMSINFO_H
#define FREEMMS_MMSINFO_H

#include <list>
#include <string>
#include <spdlog/spdlog.h>

#include "MMSV.h"
#include "MMSPart.h"
#include "Field.h"

class MMSInfo {
public:
    MMSInfo();

    ~MMSInfo();

    std::string toPlain(bool includeBody);

    bool hasBody();

    void addHeaderField(const field &f) {
        _header->push_back(f);
    }

    void addPart(MMSPart *part);

    std::list<field> *header() const;

    std::list<MMSPart *> *body() const;

private:
    std::list<field> *_header;
    std::list<MMSPart *> *_body;

};

#endif //FREEMMS_MMSINFO_H
