#ifndef FREEMMS_MMSINFO_H
#define FREEMMS_MMSINFO_H

#include <list>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

#include "MMSV.h"
#include "MMSPart.h"
#include "Field.h"

class MMSInfo {
public:
    MMSInfo();

    MMSInfo(const MMSInfo &info);

    MMSInfo(MMSInfo &&info) noexcept;

    ~MMSInfo();

    std::string toPlain(bool includeBody);

    bool hasBody();

    void addHeaderField(const field &f) {
        _header->push_back(f);
    }

    void addPart(const MMSPart &part);

    std::list<field> *header() const;

    std::list<MMSPart> *body() const;

private:
    std::list<field> *_header;
    std::list<MMSPart> *_body;

};

typedef std::shared_ptr<MMSInfo> mms_info;

#endif //FREEMMS_MMSINFO_H
