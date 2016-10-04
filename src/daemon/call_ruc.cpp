#include <sstream>
#include <cstring>
#include "judge_daemon.h"

#define RUC_NAME "../cwoj_runner "

#define SPJ_MAX_TIME 5000
#define SPJ_MAX_MEM 1073741824
#define MAX_REPORT_LENGTH 64


int run_judge(std::string target, const char *input, const char *output, int time, int mem, ExecutionStatus *info) {
	std::ostringstream command;
	static char info_str[128];
	command << RUC_NAME << target << ' ' << time
	        << ' ' << mem << " \"" << input << "\" \"" << output << "\"";
	const std::string &str = command.str();
	//puts(str.c_str());
	int ret = system(str.c_str());
	if (ret) {
		sprintf(info_str, "Error: RUC exited with %d", ret);
		applog(info_str);
		return ret;
	}
	FILE *run_info = fopen("run.SingleTestCaseResult", "r");
	char type[20];
	int val;
	if (!run_info) {
		applog("Can't open run.SingleTestCaseResult");
		return 1;
	}
	info->AdditionalInfo = nullptr;
	while (fscanf(run_info, "%s%*s", type) > 0) {
		if (!strcmp(type, "Info")) {
			fgets(info_str, 100, run_info);
			info->AdditionalInfo = new std::string(info_str);
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

ValidatorResult run_spj(char *datafile_out, char *datafile_in, int *score, char *data_dir) {
	std::ostringstream cmd;
	ValidatorResult info;
	info.ResultCode = -1;
#ifdef _WIN32
	system("del /f /q /s CaseScore.log report.log");
	CopyFile(datafile_in, "user.in", 0);
	CopyFile(datafile_out, "std.ans", 0);

	cmd << RUC_NAME << data_dir << "\\spj.exe " << SPJ_MAX_TIME << ' ' << SPJ_MAX_MEM << " \"\" \"\" -spj " << *CaseScore << " std.ans";
#else
	system("rm -f -r CaseScore.log report.log");
	system((std::string("cp ") + datafile_in + " UserName.in").c_str());
	system((std::string("cp ") + datafile_out + " std.ans").c_str());
	cmd << "test -x spj.exe || g++ '" << data_dir << "/spj.cpp' -o spj.exe";
	if (system(cmd.str().c_str())) {
		applog("Error: Cannot find spj");
		return info;
	}
	cmd.str("");
	cmd << RUC_NAME << "./spj.exe " << SPJ_MAX_TIME << ' ' << SPJ_MAX_MEM << " '' '' -spj " << *score << " std.ans";
#endif

	int ret = system(cmd.str().c_str());
	if (ret) {
		char msg[64];
		sprintf(msg, "Error: RUC for spj exited with %d", ret);
		applog(msg);
		return info;
	}

	{
		FILE *fscore = fopen("CaseScore.log", "r");
		if (!fscore) {
			applog("Error: Cannot open CaseScore.log");
			return info;
		}

		double t;
		if (1 != fscanf(fscore, "%lf", &t)) {
			applog("Error: Invalid CaseScore.log");
			fclose(fscore);
			return info;
		}
		info.ResultCode = 4; //must be 4 for spj
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
