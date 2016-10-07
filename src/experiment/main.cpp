//
// Created by zhangyutong926 on 10/8/16.
//

#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char **argv) {
	YAML::Node rootNode = YAML::LoadFile("/home/zhangyutong926/Workspace/CwojDaemon/cwojconfig.yaml");
	cout << "DataDir = " << rootNode["system"]["data_dir"].as<string>() << endl;
	cout << "TempDir = " << rootNode["system"]["temp_dir"].as<string>() << endl;
	cout << "DBHost = " << rootNode["system"]["db_host"].as<string>() << endl;
	cout << "DBUser = " << rootNode["system"]["db_user"].as<string>() << endl;
	cout << "DBPass = " << rootNode["system"]["db_pass"].as<string>() << endl;
	cout << "DBName = " << rootNode["system"]["db_name"].as<string>() << endl;
	cout << "HttpBindAddr = " << rootNode["system"]["http_bind_addr"].as<string>() << endl;
	cout << "HttpBindPort = " << rootNode["system"]["http_bind_port"].as<u_int16_t>() << endl;
	for (const YAML::Node &node : rootNode["languages"]) {
		cout << "LanguageId = " << node["id"].as<int>() << "\t";
		cout << "FileExtension = " << node["file_extension"].as<string>() << "\t";
		cout << "ExtraMemory = " << node["extra_memory"].as<uint64_t>() << "\t";
		cout << "CompilationExec = " << node["compilation_exec"].as<string>() << endl;
	}
}
