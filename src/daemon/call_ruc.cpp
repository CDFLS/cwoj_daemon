#include <sstream>
#include <cstring>
#include "conf_items.h"
#include "judge_daemon.h"

#define SPJ_MAX_TIME 5000
#define SPJ_MAX_MEM 1073741824
#define MAX_REPORT_LENGTH 64


int run_judge(const char *target, const char *input, const char *output, int time, int mem, ExecutionInfo *info) {
	std::ostringstream command;
	static char info_str[128];
	command << SystemConf.RucPath << ' ' << target << ' ' << time
	        << ' ' << mem << " \"" << input << "\" \"" << output << "\"";
	const std::string &str = command.str();
	OutputLog(str.c_str());
	//puts(str.c_str());
	int ret = system(str.c_str());
	if (ret) {
		sprintf(info_str, "Error: RUC exited with %d", ret);
		OutputLog(info_str);
		return ret;
	}
	FILE *run_info = fopen("run.info", "r");
	char type[20];
	int val;
	if (!run_info) {
		OutputLog("Can't open run.info");
		return 1;
	}
	info->Info = NULL;
	while (fscanf(run_info, "%s%*s", type) > 0) {
		if (!strcmp(type, "Info")) {
			fgets(info_str, 100, run_info);
			info->Info = info_str;
		} else if (!strcmp(type, "Time")) {
			fscanf(run_info, "%d", &val);
			info->Time = val;
		} else if (!strcmp(type, "Memory")) {
			fscanf(run_info, "%d", &val);
			info->Memory = val;
		} else if (!strcmp(type, "State")) {
			fscanf(run_info, "%d", &val);
			info->State = val;
		}
	}

	fclose(run_info);
	return ret;
}

ValidatorInfo run_spj(char *datafile_out, char *datafile_in, int *score, char *data_dir) {
	std::ostringstream cmd;
	ValidatorInfo info;
	info.Result = -1;
#ifdef _WIN32
	system("del /f /q /s score.log report.log");
	CopyFile(datafile_in, "user.in", 0);
	CopyFile(datafile_out, "std.ans", 0);

	cmd << RUC_NAME << data_dir << "\\spj.exe " << SPJ_MAX_TIME << ' ' << SPJ_MAX_MEM << " \"\" \"\" -spj " << *score << " std.ans";
#else
	system("rm -f -r score.log report.log");
	system((std::string("cp ") + datafile_in + " user.in").c_str());
	system((std::string("cp ") + datafile_out + " std.ans").c_str());
	cmd << "test -x spj.exe || g++ '" << data_dir << "/spj.cpp' -o spj.exe";
	if (system(cmd.str().c_str())) {
		OutputLog("Error: Cannot find spj");
		return info;
	}
	cmd.str("");
	cmd << SystemConf.RucPath << ' ' << "./spj.exe " << SPJ_MAX_TIME << ' ' << SPJ_MAX_MEM << " '' '' -spj " << *score << " std.ans";
#endif

	int ret = system(cmd.str().c_str());
	if (ret) {
		char msg[64];
		sprintf(msg, "Error: RUC for spj exited with %d", ret);
		OutputLog(msg);
		return info;
	}

	{
		FILE *fscore = fopen("score.log", "r");
		if (!fscore) {
			OutputLog("Error: Cannot open score.log");
			return info;
		}

		double t;
		if (1 != fscanf(fscore, "%lf", &t)) {
			OutputLog("Error: Invalid score.log");
			fclose(fscore);
			return info;
		}
		info.Result = 4; //must be 4 for spj
		*score = (int) t;
		fclose(fscore);
	}

	{
		info.UserMismatch = (char *) malloc(MAX_REPORT_LENGTH);
		info.UserMismatch[0] = '\0';
		FILE *freport = fopen("report.log", "r");
		if (freport) {
			fgets(info.UserMismatch, MAX_REPORT_LENGTH, freport);
			fclose(freport);
		}
	}

	return info;
}
