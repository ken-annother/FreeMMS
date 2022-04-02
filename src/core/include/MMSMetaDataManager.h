#ifndef FREEMMS_MMSMETADATAMANAGER_H
#define FREEMMS_MMSMETADATAMANAGER_H

#include <string>
#include <vector>

struct MetaConfig {
    std::string name;
    std::string version;
    int value;
};

class MMSMetaDataManager {
public:
    typedef std::vector<MetaConfig> MetaConfigList;
private:
    std::string configDir;
    MetaConfigList characterSetMIBENumConfig;
    MetaConfigList mmsHeaderFieldConfig;
    MetaConfigList mmsOptionContentTypeConfig;
    MetaConfigList mmsOptionDeliveryReportConfig;
    MetaConfigList mmsOptionMessageClassConfig;
    MetaConfigList mmsOptionMessageTypeConfig;
    MetaConfigList mmsOptionPriorityConfig;
    MetaConfigList mmsOptionReadReplyConfig;
    MetaConfigList mmsOptionReportAllowedConfig;
    MetaConfigList mmsOptionResponseStatusConfig;
    MetaConfigList mmsOptionParamFieldConfig;
    MetaConfigList mmsOptionParamWellknownConfig;
public:
    explicit MMSMetaDataManager(const std::string &configDir);

    std::string findFieldNameByCode(unsigned char fieldCode);

    std::string findMessageTypeNameByCode(unsigned char messageTypeCode);

    std::string findMessageClassByCode(unsigned char messageClassCode);

    std::string findPriorityByCode(unsigned char priorityCode);

    std::string findDeliveryReportByCode(unsigned char deliveryReportCode);

    std::string findReadReplyByCode(unsigned char readReplyCode);

    std::string findCharacterSetByCode(unsigned char mibeNum);

    std::string findContentTypeByCode(unsigned char contentTypeCode);

    std::string findParamWellknownByCode(unsigned char paramWellknownCode);

    std::string findParamFieldByCode(unsigned char paramFieldCode);
};


#endif //FREEMMS_MMSMETADATAMANAGER_H
