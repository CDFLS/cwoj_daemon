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
    SINT_32,
    UINT_16,
    UINT_64,
    PATH,
    PATH_STRING,
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
    std::map<void *, ConfigItemType> *SubLevelItem = nullptr;

    inline T *CastType(DaemonConfiguration *confObj, T DaemonConfiguration::* targetObj) {
        return FromOffset<DaemonConfiguration, T>(confObj, targetObj);
    }

    inline ConfigFileItem(std::string *prefix,
                          ConfigItemType type,
                          std::string key,
                          T D::*pointer,
                          std::map<void *, ConfigItemType> *sublevel):
            PrefixHier(prefix),
            ValueType(type),
            ConfigKey(key),
            DefaultConfObjItemPointer(pointer),
            SubLevelItem(sublevel) {}
};

typedef DaemonConfiguration DCF; // Default Configuration Object
std::map<void *, ConfigItemType> ConfigItemMap{
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::PATH_STRING,
                        "data_dir",
                        &DaemonConfiguration::DataDir,
                        nullptr
                ),
                ConfigItemType::PATH_STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::PATH_STRING,
                        "temp_dir",
                        &DaemonConfiguration::TempDir,
                        nullptr
                ),
                ConfigItemType::PATH_STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, boost::filesystem::path>(
                        new std::string("system"),
                        ConfigItemType::PATH,
                        "data_dir",
                        &DaemonConfiguration::DataDirectory,
                        nullptr
                ),
                ConfigItemType::PATH
        },
        {
                new ConfigFileItem<DaemonConfiguration, boost::filesystem::path>(
                        new std::string("system"),
                        ConfigItemType::PATH,
                        "temp_dir",
                        &DaemonConfiguration::TempDirectory,
                        nullptr
                ),
                ConfigItemType::PATH
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "db_host",
                        &DaemonConfiguration::DBHost,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "db_user",
                        &DaemonConfiguration::DBUser,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "db_pass",
                        &DaemonConfiguration::DBPass,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "db_name",
                        &DaemonConfiguration::DBName,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "http_bind_addr",
                        &DaemonConfiguration::HttpBindAddr,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, u_int16_t>(
                        new std::string("system"),
                        ConfigItemType::UINT_16,
                        "http_bind_port",
                        &DaemonConfiguration::HttpBindPort,
                        nullptr
                ),
                ConfigItemType::UINT_16
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::string>(
                        new std::string("system"),
                        ConfigItemType::STRING,
                        "ruc_path",
                        &DaemonConfiguration::RucPath,
                        nullptr
                ),
                ConfigItemType::STRING
        },
        {
                new ConfigFileItem<DaemonConfiguration, std::vector<ProgrammingLanguage>>(
                        new std::string("system"),
                        ConfigItemType::VECTOR,
                        "languages",
                        &DaemonConfiguration::Languages,
                        new std::map<void *, ConfigItemType> {
                                {
                                        new ConfigFileItem<ProgrammingLanguage, int>(
                                                nullptr,
                                                ConfigItemType::STRING,
                                                "id",
                                                &ProgrammingLanguage::LanguageId,
                                                nullptr
                                        ),
                                        ConfigItemType::STRING
                                },
                                {
                                        new ConfigFileItem<ProgrammingLanguage, std::string>(
                                                nullptr,
                                                ConfigItemType::STRING,
                                                "file_extension",
                                                &ProgrammingLanguage::FileExtension,
                                                nullptr
                                        ),
                                        ConfigItemType::STRING
                                },
                                {
                                        new ConfigFileItem<ProgrammingLanguage, u_int64_t>(
                                                nullptr,
                                                ConfigItemType::UINT_64,
                                                "extra_memory",
                                                &ProgrammingLanguage::ExtraMemory,
                                                nullptr
                                        ),
                                        ConfigItemType::UINT_64
                                },
                                {
                                        new ConfigFileItem<ProgrammingLanguage, std::string>(
                                                nullptr,
                                                ConfigItemType::STRING,
                                                "compilation_exec",
                                                &ProgrammingLanguage::CompilationExec,
                                                nullptr
                                        ),
                                        ConfigItemType::STRING
                                }
                        }
                ),
                ConfigItemType::VECTOR
        }
};

#endif //CWOJ_DAEMON_CONF_ITEM_MAP_H
