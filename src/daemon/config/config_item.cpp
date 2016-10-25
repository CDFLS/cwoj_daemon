//
// Created by zhangyutong926 on 10/6/16.
//
#include <string>
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "../INI1.26.h"

#include "config_item.h"
#include "../judge_daemon.h"
#include "conf_item_map.h"
#include "../util/inline_util.h"

using std::string;
using boost::filesystem::path;
typedef INI<string, string, string> SimpleIni;

DaemonConfiguration SystemConf;

DaemonConfiguration::DaemonConfiguration() :
        Languages(std::vector<ProgrammingLanguage>()),
        DebugMode(false){}

bool DaemonConfiguration::IsLanguageExists(int languageId) {
    for (ProgrammingLanguage &pl : Languages) {
        if (pl.LanguageId == languageId)
            return true;
    }
    return false;
}

ProgrammingLanguage *DaemonConfiguration::FindLanguage(int languageId) {
    for (ProgrammingLanguage &pl : Languages) {
        if (pl.LanguageId == languageId)
            return &pl;
    }
    return nullptr;
}

bool DaemonConfiguration::ReadConfiguration(std::string configFilePath) {
    if (exists(configFilePath + YAML_EXT)) {
        OutputLog("YAML file " + configFilePath + YAML_EXT +
                  " has been found with first priority. CWOJ will parse it as system configuration.");
        return ParseYaml(configFilePath + YAML_EXT);
    } else if (exists(configFilePath + INI_EXT)) {
        OutputLog("INI file " + configFilePath + INI_EXT +
                  " has been found with second priority. CWOJ will parse it as system configuration.");
        return ParseIni(configFilePath + INI_EXT);
    } else {
        OutputLog("Error: No valid configuration file found from given path.");
        return false;
    }
}

// It cannot be declared as a class private inline member function. I'm so sad, because of the restriction of recursive header-file inclusion.
template<typename B, typename O>
static inline void AssignValueFromYaml(YAML::Node rootNode, DaemonConfiguration *obj, std::pair<void* const, ConfigItemType> &pair) {
    ConfigFileItem<B, O> *item = (ConfigFileItem<B, O> *) pair.first;
    if (item->PrefixHier == nullptr) {
        *item->CastType(obj, item->DefaultConfObjItemPointer) = rootNode[item->ConfigKey].template as<O>();
    } else {
        *item->CastType(obj, item->DefaultConfObjItemPointer) = rootNode[*item->PrefixHier][item->ConfigKey].template as<O>();
    }
}

bool DaemonConfiguration::ParseYaml(std::string path) {
    YAML::Node rootNode = YAML::LoadFile(path);

    for (std::pair<void* const, ConfigItemType> &pair : ConfigItemMap) {
        if (pair.second != ConfigItemType::VECTOR) {
            switch (pair.second) {
                case PATH_STRING: // FIXME This is a special case that we cannot just simply apply AssignValue
                    ConfigFileItem<DCF, string> *pathItem = (ConfigFileItem<DCF, string> *) pair.first;
                    if (pathItem->PrefixHier == nullptr) {
                        boost::filesystem::path path(rootNode[pathItem->ConfigKey].as<std::string>());
                        *pathItem->CastType(this, pathItem->DefaultConfObjItemPointer) = path.string();
                    } else {
                        boost::filesystem::path path(rootNode[*pathItem->PrefixHier][pathItem->ConfigKey].as<std::string>());
                        *pathItem->CastType(this, pathItem->DefaultConfObjItemPointer) = path.string();
                    }
                    break;
                case PATH: // FIXME This is also a special case
                    ConfigFileItem<DCF, boost::filesystem::path> *pathStringItem = (ConfigFileItem<DCF, boost::filesystem::path> *) pair.first;
                    if (pathStringItem->PrefixHier == nullptr) {
                        boost::filesystem::path path(rootNode[pathStringItem->ConfigKey].as<std::string>());
                        *pathStringItem->CastType(this, pathStringItem->DefaultConfObjItemPointer) = path;
                    } else {
                        boost::filesystem::path path(rootNode[*pathStringItem->PrefixHier][pathStringItem->ConfigKey].as<std::string>());
                        *pathStringItem->CastType(this, pathStringItem->DefaultConfObjItemPointer) = path;
                    }
                    break;
                case STRING:
//                    ConfigFileItem<DCF, string> *stringItem = (ConfigFileItem<DCF, string> *) pair.first;
//                    if (stringItem->PrefixHier == nullptr) {
//                        *stringItem->CastType(this, stringItem->DefaultConfObjItemPointer) = rootNode[stringItem->ConfigKey].as<std::string>();
//                    } else {
//                        *stringItem->CastType(this, stringItem->DefaultConfObjItemPointer) = rootNode[*stringItem->PrefixHier][stringItem->ConfigKey].as<std::string>();
//                    }
                    AssignValueFromYaml<DCF, string>(rootNode, this, pair);
                    break;
                case BOOL:
//                    ConfigFileItem<DCF, bool> *boolItem = (ConfigFileItem<DCF, bool> *) pair.first;
//                    if (boolItem->PrefixHier == nullptr) {
//                        *boolItem->CastType(this, boolItem->DefaultConfObjItemPointer) = rootNode[boolItem->ConfigKey].as<bool>();
//                    } else {
//                        *boolItem->CastType(this, boolItem->DefaultConfObjItemPointer) = rootNode[*boolItem->PrefixHier][boolItem->ConfigKey].as<bool>();
//                    }
                    AssignValueFromYaml<DCF, bool>(rootNode, this, pair);
                    break;
                case SINT_32:
                    AssignValueFromYaml<DCF, int32_t>(rootNode, this, pair);
                    break;
                case UINT_16:
                    AssignValueFromYaml<DCF, u_int16_t>(rootNode, this, pair);
                    break;
                case UINT_64:
                    AssignValueFromYaml<DCF, u_int64_t>(rootNode, this, pair);
                    break;
                default:
                    return false;
            }
        } else {

        }
    }

    return true;

//    TempDirectory = boost::filesystem::path((string) rootNode["system"]["temp_dir"].as<string>());
//    DataDirectory = boost::filesystem::path((string) rootNode["system"]["data_dir"].as<string>());
//    TempDir = TempDirectory.string();
//    DataDir = DataDirectory.string();
//
//    UserName = rootNode["system"]["user_name"].as<string>();
//    DBHost = rootNode["system"]["db_host"].as<string>();
//    DBUser = rootNode["system"]["db_user"].as<string>();
//    DBPass = rootNode["system"]["db_pass"].as<string>();
//    DBName = rootNode["system"]["db_name"].as<string>();
//    HttpBindAddr = rootNode["system"]["http_bind_addr"].as<string>();
//    HttpBindPort = rootNode["system"]["http_bind_port"].as<u_int16_t>();
//    RucPath = rootNode["system"]["ruc_path"].as<string>();
//    for (const YAML::Node &node : rootNode["languages"]) {
//        ProgrammingLanguage pl;
//        pl.LanguageId = node["id"].as<int>() - 1;
//        pl.FileExtension = node["file_extension"].as<string>();
//        pl.ExtraMemory = node["extra_memory"].as<u_int64_t>();
//        pl.CompilationExec = node["compilation_exec"].as<string>();
//        SystemConf.Languages.push_back(pl);
//    }
//    OutputLog("Ip Address = " + HttpBindAddr);
//    OutputLog("Port = " + HttpBindPort);


}

bool DaemonConfiguration::ParseIni(std::string path) {
    SimpleIni ini(path, false);
    if (!ini.Parse()) {
        OutputLog("Error: Cannot open config file " + path + ", Exit...");
        exit(1);
    }

    ini.Select("system");
    std::string tmp = ini.Get(std::string("datadir"), std::string(""));
    if (tmp == "") {
        OutputLog("Error: We don't know your data directory.");
        return false;
    }

    DataDir = string(tmp);
    tmp = ini.Get(std::string("DATABASE_HOST"), std::string("localhost"));
    DBHost = string(tmp);
    tmp = ini.Get(std::string("DATABASE_USER"), std::string("root"));
    DBUser = string(tmp);
    tmp = ini.Get(std::string("DATABASE_PASS"), std::string(""));
    DBPass = string(tmp);
    tmp = ini.Get(std::string("DATABASE_NAME"), std::string("cwoj"));
    DBName = string(tmp);
    tmp = ini.Get(std::string("HTTP_BIND_IP"), std::string("0.0.0.0"));
    HttpBindAddr = string(tmp);
    tmp = ini.Get(std::string("TempDir"), std::string(""));
    TempDir = string(tmp);
    HttpBindPort = ini.Get<const char *, unsigned short>("HTTP_BIND_PORT", 8881u);
    tmp = ini.Get(string("RUC_PATH"), string(DEFAULT_RUC));

    OutputLog("HttpBindAddr = " + HttpBindAddr);

    for (auto i = ini.sections.begin(); i != ini.sections.end(); ++i) {
        const string &lang = i->first;
        if (lang.find("lang") != 0)
            continue;
        int num = atoi(lang.c_str() + 4);
        if (!num || num > MAXLANG) {
            OutputLog("Info: Language number is not correct.");
            return false;
        }
        num--;
        ProgrammingLanguage pl;
        std::map<string, string> &keys = *(i->second);
        if (keys.count(string("extra_mem")))
            pl.ExtraMemory = (uint64_t) Convert<int>(keys[string("extra_mem")]);
        else
            pl.ExtraMemory = 0;
        if (keys.count(string("compiler")))
            pl.CompilationExec = keys[string("compiler")];
        else
            return false;
        if (keys.count(string("ext")))
            pl.FileExtension = keys[string("ext")];
        else
            return false;
        pl.LanguageId = num;
        Languages.push_back(pl);
    }

    return true;
}

//DaemonConfiguration SystemConf {
//	if (SystemConf == nullptr) {
//		SystemConf = new DaemonConfiguration();
//	}
//	return *SystemConf;
//}
