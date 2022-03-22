#include "MMSMetaDataManager.h"

#include <fstream>
#include <sstream>
#include <algorithm>

#include "json.hpp"

using namespace std;
using namespace nlohmann;

static void parseConfigFile(const std::string &filePath, MMSMetaDataManager::MetaConfigList &configList) {
    ifstream s(filePath);
    if (!s.is_open()) {
        throw runtime_error("file not exist");
    }

    json dataJson;
    s >> dataJson;
    for (auto &obj: dataJson) {
        MetaConfig config;
        if (obj.contains("VERSION")) {
            config.version = obj.at("VERSION").get<string>();
        }
        config.name = obj.at("NAME").get<string>();
        config.value = obj.at("VALUE").get<int>();
        configList.push_back(config);
    }
}

MMSMetaDataManager::MMSMetaDataManager(const std::string &configDir) : configDir(configDir) {
    parseConfigFile(configDir + "/character_sets_mibenum.json", this->characterSetMIBENumConfig);
    parseConfigFile(configDir + "/mms_header_field.json", this->mmsHeaderFieldConfig);
    parseConfigFile(configDir + "/mms_option_content_type.json", this->mmsOptionContentTypeConfig);
    parseConfigFile(configDir + "/mms_option_delivery_report.json", this->mmsOptionDeliveryReportConfig);
    parseConfigFile(configDir + "/mms_option_message_class.json", this->mmsOptionMessageClassConfig);
    parseConfigFile(configDir + "/mms_option_message_type.json", this->mmsOptionMessageTypeConfig);
    parseConfigFile(configDir + "/mms_option_priority.json", this->mmsOptionPriorityConfig);
    parseConfigFile(configDir + "/mms_option_read_reply.json", this->mmsOptionReadReplyConfig);
    parseConfigFile(configDir + "/mms_option_report_allowed.json", this->mmsOptionReportAllowedConfig);
    parseConfigFile(configDir + "/mms_option_response_status.json", this->mmsOptionResponseStatusConfig);
    parseConfigFile(configDir + "/mms_param_field.json", this->mmsOptionParamFieldConfig);
    parseConfigFile(configDir + "/mms_param_wellknown.json", this->mmsOptionParamWellknownConfig);
}

std::string MMSMetaDataManager::findFieldNameByCode(unsigned char fieldCode) {
    int markCode = fieldCode & 0x7F;

    auto it = find_if(this->mmsHeaderFieldConfig.begin(), this->mmsHeaderFieldConfig.end(),
                      [markCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == markCode;
                      });
    if (it != mmsHeaderFieldConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findMessageTypeNameByCode(unsigned char messageTypeCode) {
    auto it = find_if(this->mmsOptionMessageTypeConfig.begin(), this->mmsOptionMessageTypeConfig.end(),
                      [messageTypeCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == messageTypeCode;
                      });
    if (it != mmsOptionMessageTypeConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findMessageClassByCode(unsigned char messageClassCode) {
    auto it = find_if(this->mmsOptionMessageClassConfig.begin(), this->mmsOptionMessageClassConfig.end(),
                      [messageClassCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == messageClassCode;
                      });
    if (it != mmsOptionMessageClassConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findPriorityByCode(unsigned char priorityCode) {
    auto it = find_if(this->mmsOptionPriorityConfig.begin(), this->mmsOptionPriorityConfig.end(),
                      [priorityCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == priorityCode;
                      });
    if (it != mmsOptionPriorityConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findDeliveryReportByCode(unsigned char deliveryReportCode) {
    auto it = find_if(this->mmsOptionDeliveryReportConfig.begin(), this->mmsOptionDeliveryReportConfig.end(),
                      [deliveryReportCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == deliveryReportCode;
                      });
    if (it != mmsOptionDeliveryReportConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findReadReplyByCode(unsigned char readReplyCode) {
    auto it = find_if(this->mmsOptionReadReplyConfig.begin(), this->mmsOptionReadReplyConfig.end(),
                      [readReplyCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == readReplyCode;
                      });
    if (it != mmsOptionReadReplyConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findCharacterSetByCode(unsigned char mibeNum) {
    auto it = find_if(this->characterSetMIBENumConfig.begin(), this->characterSetMIBENumConfig.end(),
                      [mibeNum](const MetaConfig &rhs) -> bool {
                          return rhs.value == mibeNum;
                      });
    if (it != characterSetMIBENumConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findContentTypeByCode(unsigned char contentTypeCode) {
    auto it = find_if(this->mmsOptionContentTypeConfig.begin(), this->mmsOptionContentTypeConfig.end(),
                      [contentTypeCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == contentTypeCode;
                      });
    if (it != mmsOptionContentTypeConfig.end()) {
        return it->name;
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findParamWellknownByCode(unsigned char paramWellknownCode) {
    auto it = find_if(this->mmsOptionParamWellknownConfig.begin(), this->mmsOptionParamWellknownConfig.end(),
                      [paramWellknownCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == paramWellknownCode;
                      });
    if (it != mmsOptionParamWellknownConfig.end()) {
        if (it->version.length() == 0) {
            return it->name;
        } else {
            return it->name + "," + it->version;
        }
    } else {
        return "";
    }
}

std::string MMSMetaDataManager::findParamFieldByCode(unsigned char paramFieldCode) {
    auto it = find_if(this->mmsOptionParamFieldConfig.begin(), this->mmsOptionParamFieldConfig.end(),
                      [paramFieldCode](const MetaConfig &rhs) -> bool {
                          return rhs.value == paramFieldCode;
                      });
    if (it != mmsOptionParamFieldConfig.end()) {
        if (it->version.length() == 0) {
            return it->name;
        } else {
            return it->name + "," + it->version;
        }
    } else {
        return "";
    }
}
