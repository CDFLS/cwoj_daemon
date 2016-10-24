//
// Created by zhangyutong926 on 10/24/16.
//

#ifndef CWOJ_DAEMON_CONF_ITEM_MAP_H
#define CWOJ_DAEMON_CONF_ITEM_MAP_H

#include <map>
#include <vector>
#include <string>

#include "config_item.h"

extern DaemonConfiguration SystemConf;

enum ConfigItemType {
    STRING,
    SINT, // Signed int
    UINT, // Unsigned int
    PATH,
    BOOL,
    VECTOR
};

class ConfigFileItem {
public:
    ConfigItemType ValueType;
    std::string ConfigKey; // e.g. "ini" -> "DATABASE_USER", "yaml" -> "db_user"
    void DaemonConfiguration::*DefaultConfObjItemPointer;
    ConfigFileItem *SubLevelItem = nullptr;
};

std::vector<ConfigFileItem> ConfigItemList {
        ConfigFileItem {
                ConfigItemType::STRING,
                "data_dir",
                &DaemonConfiguration::DataDir,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "temp_dir",
                &DaemonConfiguration::TempDir,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "db_host",
                &DaemonConfiguration::DBHost,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "db_user",
                &DaemonConfiguration::DBUser,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "db_pass",
                &DaemonConfiguration::DBPass,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "db_name",
                &DaemonConfiguration::DBName,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "http_bind_addr",
                &DaemonConfiguration::HttpBindAddr,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "http_bind_port",
                &DaemonConfiguration::HttpBindPort,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::STRING,
                "ruc_path",
                &DaemonConfiguration::RucPath,
                nullptr
        },
        ConfigFileItem {
                ConfigItemType::VECTOR,
                "languages",
                &DaemonConfiguration::Languages,
                nullptr
        }
};

#endif //CWOJ_DAEMON_CONF_ITEM_MAP_H
