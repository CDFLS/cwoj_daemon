//
// Created by zhangyutong926 on 10/24/16.
//

#ifndef CWOJ_DAEMON_CONF_ITEM_MAP_H
#define CWOJ_DAEMON_CONF_ITEM_MAP_H

#include <map>
#include <vector>
#include <string>

#include "config_item.h"
#include "../util/inline_util.h"

extern DaemonConfiguration SystemConf;

enum ConfigItemType {
    STRING,
    SINT, // Signed int
    UINT, // Unsigned int
    PATH,
    BOOL,
    VECTOR
};

template<typename D, typename T>
class ConfigFileItem {
public:
    std::string *PrefixHier = nullptr; // nullptr means no prefix hierarchy
    ConfigItemType ValueType;
    std::string ConfigKey;
    T D::*DefaultConfObjItemPointer;
    std::vector<void *> *SubLevelItem = nullptr;

    inline T *CastType(DaemonConfiguration *confObj, T DaemonConfiguration::* targetObj) {
        return FromOffset<DaemonConfiguration, T>(confObj, targetObj);
    }

    inline ConfigFileItem(std::string *prefix,
                          ConfigItemType type,
                          std::string key,
                          T D::*pointer,
                          std::vector<void *> *sublevel):
            PrefixHier(prefix),
            ValueType(type),
            ConfigKey(key),
            DefaultConfObjItemPointer(pointer),
            SubLevelItem(sublevel) {}
};

std::vector<void *> ConfigItemList{
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "data_dir",
                &DaemonConfiguration::DataDir,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "temp_dir",
                &DaemonConfiguration::TempDir,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "db_host",
                &DaemonConfiguration::DBHost,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "db_user",
                &DaemonConfiguration::DBUser,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "db_pass",
                &DaemonConfiguration::DBPass,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "db_name",
                &DaemonConfiguration::DBName,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "http_bind_addr",
                &DaemonConfiguration::HttpBindAddr,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, u_int16_t>(
                new std::string("system"),
                ConfigItemType::UINT,
                "http_bind_port",
                &DaemonConfiguration::HttpBindPort,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::string>(
                new std::string("system"),
                ConfigItemType::STRING,
                "ruc_path",
                &DaemonConfiguration::RucPath,
                nullptr
        ),
        new ConfigFileItem<DaemonConfiguration, std::vector<ProgrammingLanguage>>(
                new std::string("system"),
                ConfigItemType::VECTOR,
                "languages",
                &DaemonConfiguration::Languages,
                new std::vector<void *> {
                        new ConfigFileItem<ProgrammingLanguage, int>(
                                nullptr,
                                ConfigItemType::STRING,
                                "id",
                                &ProgrammingLanguage::LanguageId,
                                nullptr
                        ),
                        new ConfigFileItem<ProgrammingLanguage, std::string>(
                                nullptr,
                                ConfigItemType::STRING,
                                "file_extension",
                                &ProgrammingLanguage::FileExtension,
                                nullptr
                        ),
                        new ConfigFileItem<ProgrammingLanguage, u_int64_t>(
                                nullptr,
                                ConfigItemType::UINT,
                                "extra_memory",
                                &ProgrammingLanguage::ExtraMemory,
                                nullptr
                        ),
                        new ConfigFileItem<ProgrammingLanguage, std::string>(
                                nullptr,
                                ConfigItemType::STRING,
                                "compilation_exec",
                                &ProgrammingLanguage::CompilationExec,
                                nullptr
                        ),
                }
        )
};

#endif //CWOJ_DAEMON_CONF_ITEM_MAP_H
