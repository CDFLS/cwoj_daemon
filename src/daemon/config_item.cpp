//
// Created by zhangyutong926 on 10/6/16.
//
#include <string>
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "INI1.26.h"

#include "conf_items.h"
#include "judge_daemon.h"

using std::string;
using namespace boost::filesystem;
typedef INI<string, string, string> SimpleIni;

DaemonConfiguration::DaemonConfiguration():
		Languages(std::vector<ProgrammingLanguage>()) {}

DaemonConfiguration::~DaemonConfiguration() {}

bool DaemonConfiguration::ReadConfiguration(std::string configFilePath) {
	if (exists(configFilePath + INI_EXT)) {
		return ParseIni(configFilePath + INI_EXT);
	} else if (exists(configFilePath + YAML_EXT)) {
		return ParseYaml(configFilePath + YAML_EXT);
	} else {
		OutputLog("Error: No configuration file found from given path.");
		return false;
	}
}

bool DaemonConfiguration::ParseYaml(std::string path) {
	YAML::Node rootNode = YAML::LoadFile(path);
	DataDir = rootNode["System"]["DataDir"].as<string>();
	TempDir = rootNode["System"]["TempDir"].as<string>();
	DBHost = rootNode["System"]["DatabaseHost"].as<string>();
	DBUser = rootNode["System"]["DatabaseUser"].as<string>();
	DBPass = rootNode["System"]["DatabasePass"].as<string>();
	DBName = rootNode["System"]["DatabaseName"].as<string>();
	for (const auto &node : rootNode) {
		if (node.Tag() == "System") continue;
		if (node.Tag().find("Language") == string::npos) continue;
		ProgrammingLanguage pl;
		string tag = node.Tag();
		string::size_type firstDigit = string::npos;
		for (string::size_type cp = tag.size() - 1; cp >= 0; cp--) {
			if (!isdigit(tag[cp])) {
				firstDigit = cp + 1;
				break;
			}
		}
		pl.LanguageId = atoi(tag.substr(firstDigit).c_str());
		pl.FileExtension = node["FileExtension"].as<string>();
		pl.ExtraMemory = node["ExtraMemory"].as<uint64_t>();
		pl.CompilationExec = node["CompilationExec"].as<string>();
	}
	return false;
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

	return false;
}

DaemonConfiguration DaemonConfiguration::GetInstance() {
	if (SingletonInstance == nullptr) {
		SingletonInstance = new DaemonConfiguration;
	}
	return *SingletonInstance;
}
