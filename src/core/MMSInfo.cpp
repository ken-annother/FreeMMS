#include "MMSInfo.h"
#include <list>
#include <algorithm>
#include <sstream>

using namespace std;

MMSInfo::MMSInfo() : _header(new std::list<field>()), _body(new std::list<MMSPart>) {
}


MMSInfo::MMSInfo(const MMSInfo &info) {
    _header = new list<field>();
    _header->assign(info._header->begin(), info._header->end());

    _body = new list<MMSPart>;
    _body->assign(info._body->begin(), info._body->end());
}

MMSInfo::MMSInfo(MMSInfo &&info) noexcept {
    _header = info._header;
    _body = info._body;

    info._header = nullptr;
    info._body = nullptr;
}

MMSInfo::~MMSInfo() {
    delete _header;
    delete _body;
}

#define NLRF "\r\n"
#define PART_SEPARATOR "----------------------------part"
#define PART_SEPARATOR_END "----------------------------part--"


std::string MMSInfo::toPlain(bool includeBody) {
    stringstream ss;

    for (auto &f: *_header) {
        ss << f.name.value << ": " << f.value.value << NLRF;
    }

    ss << NLRF;

    if (hasBody()) {
        for (auto &part: *_body) {
            ss << PART_SEPARATOR << NLRF;

            for (auto &f: part.header()) {
                ss << f.name.value << ": " << f.value.value << NLRF;
            }
            ss << "Content-Length: " << part.dataLen() << NLRF;
            if (includeBody) {
                ss.write(part.data(), part.dataLen());
                ss << NLRF;
            }
        }

        ss << PART_SEPARATOR_END;
    }

    return ss.str();
}

bool MMSInfo::hasBody() {
    auto it = find_if(_header->begin(), _header->end(), [](const field &rhs) -> bool {
        return rhs.name.value == "Message-Type" && rhs.value.value == "M-Retrieve-Conf";
    });
    return it != _header->end();
}

void MMSInfo::addPart(const MMSPart &part) {
    spdlog::info("MMSInfo::addPart, part[{}]", (void *) &part);
    this->_body->push_back(part);
    spdlog::info("MMSInfo::addPart, -----------");
}

std::list<field> *MMSInfo::header() const {
    return _header;
}

std::list<MMSPart> *MMSInfo::body() const {
    return _body;
}
