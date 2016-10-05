#ifndef CONF_ITEM_H_
#define CONF_ITEM_H_

#define CONFIG_FILE_NO_EXT "/etc/cwojconfig"
#define INI_EXT ".ini"
#define YAML_EXT ".yaml"
#define MAXLANG 100

#include <string>
#include <vector>

//Configure variables defined in readconf.cpp

extern char DATABASE_HOST[], DATABASE_USER[], DATABASE_PASS[], DATABASE_NAME[];
extern char HTTP_BIND_IP[];
extern uint16_t HTTP_BIND_PORT;

extern bool lang_exist[];
extern int lang_extra_mem[];
extern std::string lang_ext[], lang_compiler[];

extern char DataDir[];
extern char TempDir[];

// FIXME Perform a full-scale test to this new configuration system
// FIXME Migrate system configuration from the extern variables above to the following new classes
class ProgrammingLanguage {
public:
	int LanguageId;
	std::string FileExtension, CompilationExec;
	uint64_t ExtraMemory;
};

class DaemonConfiguration {
public:
	std::string DBHost, DBUser, DBPass, DBName, HttpBindAddr, DataDir, TempDir;
	u_int16_t HttpBindPort;
	std::vector<ProgrammingLanguage> Languages;

	bool ReadConfiguration(std::string = std::string(CONFIG_FILE_NO_EXT));

	static DaemonConfiguration GetInstance();

private:
	static DaemonConfiguration *SingletonInstance;

	DaemonConfiguration();

	~DaemonConfiguration();

	bool ParseIni(std::string);

	bool ParseYaml(std::string);
};

#endif
