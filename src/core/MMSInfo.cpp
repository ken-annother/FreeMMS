#include "MMSInfo.h"
#include <spdlog/spdlog.h>
#include <list>
#include <algorithm>
#include <sstream>

using namespace std;

MMSInfo::MMSInfo() : _header(new std::list<field>()), _body(new std::list<MMSPart *>) {
    spdlog::info("MMSInfo construct ~");
}

MMSInfo::~MMSInfo() {
    spdlog::info("MMSInfo deconstruct ~");
    delete _header;
    for (auto &part: *_body) {
        delete part;
    }
    delete _body;
}

#define PART_SEPARATOR "----------------------------part"
#define NLRF "\r\n"

std::string MMSInfo::toPlain() {
    stringstream ss;

    for (auto &f: *_header) {
        ss << f.name.value << ": " << f.value.value << NLRF;
    }

    ss << NLRF;

    if (hasBody()) {
        for (auto &part: *_body) {
            ss << PART_SEPARATOR << NLRF;

            for (auto &f: part->header()) {
                ss << f.name.value << ": " << f.value.value << NLRF;
            }
            ss << "Content-Length: " << part->dataLen() << NLRF;
            ss << NLRF;
        }
    }


    return ss.str();
}

bool MMSInfo::hasBody() {
    auto it = find_if(_header->begin(), _header->end(), [](const field &rhs) -> bool {
        return rhs.name.value == "Message-Type" && rhs.value.value == "M-Retrieve-Conf";
    });
    return it != _header->end();
}

void MMSInfo::addPart(MMSPart *part) {
    this->_body->push_back(part);
}

std::list<field> *MMSInfo::header() const {
    return _header;
}

std::list<MMSPart *> *MMSInfo::body() const {
    return _body;
}
