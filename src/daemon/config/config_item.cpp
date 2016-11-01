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
        DebugMode(false) {}

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

#define ASSIGN_VALUE_FROM_YAML(B, O, rootNode, obj, pair) \
    { \
        ConfigFileItem<B, O> *item = (ConfigFileItem<B, O> *) pair.first; \
        if (item->PrefixHier == nullptr) { \
            *item->CastType(obj, item->DefaultConfObjItemPointer) = rootNode[item->ConfigKey].template as<O>(); \
        } else { \
            *item->CastType(obj, item->DefaultConfObjItemPointer) = rootNode[*item->PrefixHier][item->ConfigKey].template as<O>(); \
        } \
    }

template<typename D>
static void AssignValue(D *confObj, YAML::Node node, ConfigItemType type, std::pair<void *const, ConfigItemType> &pair) {
    switch (pair.second) {
        case BOOL:
            ASSIGN_VALUE_FROM_YAML(D, bool, node, confObj, pair);
            break;
        case LANG_INT:
            ASSIGN_VALUE_FROM_YAML(D, int, node, confObj, pair);
            break;
        case SINT_32:
            ASSIGN_VALUE_FROM_YAML(D, int32_t, node, confObj, pair);
            break;
        case UINT_16:
            ASSIGN_VALUE_FROM_YAML(D, u_int16_t, node, confObj, pair);
            break;
        case UINT_64:
            ASSIGN_VALUE_FROM_YAML(D, u_int64_t, node, confObj, pair);
            break;
        case STRING:
        ASSIGN_VALUE_FROM_YAML(D, string, node, confObj, pair);
//            ConfigFileItem<D, string> *item = (ConfigFileItem<D, string> *) pair.first;
//            if (item->PrefixHier == nullptr) {
//                *item->CastType(confObj, item->DefaultConfObjItemPointer) = node[item->ConfigKey].template as<string>();
////                OutputLog(item->ConfigKey);
//            } else {
//                *item->CastType(confObj, item->DefaultConfObjItemPointer) = node[*item->PrefixHier][item->ConfigKey].template as<string>();
////                OutputLog(*item->PrefixHier + " " + item->ConfigKey);
//            }
            break;
        default:
            if (pair.second == ConfigItemType::PATH_OBJ) {
                ConfigFileItem<D, boost::filesystem::path> *pathStringItem = (ConfigFileItem<D, boost::filesystem::path> *) pair.first;
                if (pathStringItem->PrefixHier == nullptr) {
                    boost::filesystem::path path(node[pathStringItem->ConfigKey].template as<std::string>());
                    *pathStringItem->CastType(confObj, pathStringItem->DefaultConfObjItemPointer) = path;
                } else {
                    boost::filesystem::path path(
                            node[*pathStringItem->PrefixHier][pathStringItem->ConfigKey].template as<std::string>());
                    *pathStringItem->CastType(confObj, pathStringItem->DefaultConfObjItemPointer) = path;
                }
            } else if (pair.second == ConfigItemType::PATH_STRING) {
                ConfigFileItem<D, string> *pathItem = (ConfigFileItem<D, string> *) pair.first;
                if (pathItem->PrefixHier == nullptr) {
                    boost::filesystem::path path(node[pathItem->ConfigKey].template as<std::string>());
                    *pathItem->CastType(confObj, pathItem->DefaultConfObjItemPointer) = path.string();
                } else {
                    boost::filesystem::path path(
                            node[*pathItem->PrefixHier][pathItem->ConfigKey].template as<std::string>());
                    *pathItem->CastType(confObj, pathItem->DefaultConfObjItemPointer) = path.string();
                }
            } else {
                OutputLog(
                        "Warning: You might used double-nested YAML config file, which is not supported so far, this assignment procedure will be therefore skipped.");
            }
    }
}

bool DaemonConfiguration::ParseYaml(std::string path) {
    YAML::Node rootNode = YAML::LoadFile(path);

    for (std::pair<void *const, ConfigItemType> &pair : ConfigItemMap) {
        if (pair.second != ConfigItemType::VECTOR) {
            AssignValue<DaemonConfiguration>(this, rootNode, pair.second, pair);
        } else {
            ConfigFileItem<DCF, std::vector<ProgrammingLanguage>> *sublevelRoot = (ConfigFileItem<DCF, std::vector<ProgrammingLanguage>> *) pair.first;
            for (YAML::Node subNode : rootNode[sublevelRoot->ConfigKey]) {
                ProgrammingLanguage *pl = new ProgrammingLanguage();
                for (std::pair<void *const, ConfigItemType> &sublevelPair : *sublevelRoot->SubLevelItem) {
                    AssignValue<ProgrammingLanguage>(pl, subNode, sublevelPair.second, sublevelPair);
                }
                Languages.push_back(*pl);
            }
        }
    }

    for (ProgrammingLanguage &pl : SystemConf.Languages) {
        pl.LanguageId--;
    }

    return true;
}

bool DaemonConfiguration::ParseIni(std::string path) {
    SimpleIni ini(path, false);
    if (!ini.Parse()) {
        OutputLog("Error: Cannot open config file " + path + ", Exit...");
        exit(1);
    }

    ini.Select("log");
    std::string tmp = ini.Get(std::string("NORMAL_LOG_FILE"), std::string("localhost"));
    NormalLogFile = boost::filesystem::path(string(tmp));
    tmp = ini.Get(std::string("EXCEPTION_LOG_FILE"), std::string("localhost"));
    ExceptionLogFile = boost::filesystem::path(string(tmp));

    ini.Select("system");
    tmp = ini.Get(std::string("datadir"), std::string(""));
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
