#include "judge_daemon.h"
#include "conf_items.h"
#include <string.h>
#include <dirent.h>
#include <sys/param.h>
#include <sstream>
#include <algorithm>

// from Stack Overflow:
//     https://stackoverflow.com/questions/35007134/c-boost-undefined-reference-to-boostfilesystemdetailcopy-file
#define BOOST_NO_CXX11_SCOPED_ENUMS

#include <boost/filesystem.hpp>

#undef BOOST_NO_CXX11_SCOPED_ENUMS

using namespace std;
using namespace boost::filesystem;

#ifdef __MINGW32__
#include <windows.h>
#include <boost/thread.hpp>
namespace std {
	using boost::mutex;
	using boost::lock_guard;
	using boost::unique_lock;
}
#else

#endif

//#define USE_CENA_VALIDATOR


enum {
	CMP_tra, CMP_float, CMP_int, CMP_spj
};

bool clean_files() throw() {
	int ret;
#ifdef _WIN32
	ret = system("del /f /q /s *");
#else
	ret = system("rm -f -r *");
#endif
	return ret == 0;
}

solution::solution() {
	TargetPath = getTargetPath();
	QueryMutex = new std::mutex;
	ProblemFK = ComparisonMode = LanguageType = TimeLimit = MemoryLimit = SolutionScore = ErrorCode = 0;
	IsCodeOpenSourced = 0;
	TimeStamp = 0;
	SolutionType = JUDGE_ACTION_NORMAL;
}

solution::~solution() {
	delete QueryMutex;
}

void solution::CloneFrom(const solution &from) throw() {
	ProblemFK = from.ProblemFK;
	ComparisonMode = from.ComparisonMode;
	LanguageType = from.LanguageType;
	TimeLimit = from.TimeLimit;
	MemoryLimit = from.MemoryLimit;
	SolutionScore = from.SolutionScore;
	SolutionType = from.SolutionType;
	TargetPath = from.TargetPath;
}

bool solution::Compile() throw(const char *) {
	puts("compile");
	if (!SystemConf.IsLanguageExists(LanguageType)) {
		throw "Language doesn't exist";
	}
	std::string filename("target.");
	filename += SystemConf.IsLanguageExists(LanguageType);

	FILE *code_file = fopen(filename.c_str(), "wb");
	if (code_file == NULL) {
		throw "Write code file failed";
	}

	int size = SourceCode.size();
	fwrite(SourceCode.c_str(), 1, size, code_file);
	fclose(code_file);

#ifdef _WIN32
	int ret = system((std::string("..\\win32_run_compiler.exe \"") + lang_compiler[lang] + ' ' + filename + '"').c_str());
	if (ret) {
		//printf("Can not run compiler using %s\n", (std::string("win32_run_compiler.exe ") + lang_compiler[lang] + ' ' + filename).c_str());
		throw "Can not run compiler";
	}
#else
	std::string command(SystemConf.FindLanguage(LanguageType)->CompilationExec);
	command += ' ' + filename + " >err.out 2>&1 && echo @~good~@ >err.out";
	//puts(command.c_str());
	system(command.c_str());
	OutputLog("Compilation Command = " + command);
#endif
	FILE *output = fopen("err.out", "r");
	if (!output) {
		throw "Can't open compiler output";
	}
	char *buffer = new char[65536];
	if (!buffer) {
		throw "Failed to allocate buffer.";
	}
	int read_size = fread(buffer, 1, 65400, output);
	buffer[read_size] = '\0';
	fclose(output);

	if (code_file = fopen("target.exe", "rb")) {
		fclose(code_file);
		if (strstr(buffer, "@~good~@") == NULL) {
			OutputLog("Info: Execute file exists, but compiler doesn't return 0.");
		}
		delete[] buffer;
	} else {
		if (strstr(buffer, "@~good~@") != NULL) {
			delete[] buffer;
			throw "Compiler returned 0, but execute file doesn't exist.";
		} else {
			LastState = buffer;
			SolutionScore = TimeLimit = MemoryLimit = 0;
			ErrorCode = SOLUTION_COMPILATION_ERROR;
			puts("Compile Error");
			delete[] buffer;

			OutputLog("LastState = " + LastState);

			std::unique_lock<std::mutex> Lock(*(std::mutex *) QueryMutex);
			TestCaseDetail.push_back({SOLUTION_COMPILATION_ERROR, 0, 0, LastState, 0});

			return false;
		}
	}
	return true;
}

void solution::Judge() throw(const char *) {
	// Test Code Start -- Yoto
	string testOut;
	testOut += "ProblemFK = " + to_string(ProblemFK) + "\n";
	testOut += "ComparisonMode = " + to_string(ComparisonMode) + "\n";
	testOut += "LanguageType = " + to_string(LanguageType) + "\n";
	testOut += "MemoryLimit = " + to_string(MemoryLimit) + "\n";
	testOut += "SolutionScore = " + to_string(SolutionScore) + "\n";
	testOut += "ErrorCode = " + to_string(ErrorCode) + "\n";
	testOut += "IsCodeOpenSourced = " + to_string(IsCodeOpenSourced) + "\n";
	testOut += "SolutionType = " + to_string(SolutionType) + "\n";
	testOut += "UserName = " + UserName + "\n";
	testOut += "Key = " + Key + "\n";
	testOut += "LastState = " + LastState + "\n";
	testOut += "TimeStamp = " + to_string(TimeStamp) + "\n";
	testOut += "TargetPath = " + string(TargetPath) + "\n";

	int counter = 0;
	for (SingleTestCaseReport &tc : TestCaseDetail) {
		testOut += "TestCaseDetails[" + to_string(counter) + "] = { ";
		testOut += "ErrorCode = " + to_string(tc.ErrorCode) + ", ";
		testOut += "TimeProfile = " + to_string(tc.TimeProfile) + ", ";
		testOut += "MemoryProfile = " + to_string(tc.MemoryProfile) + ", ";
		testOut += "AdditionalInformation" + tc.AdditionalInformation + ", ";
		testOut += "CaseScore" + to_string(tc.CaseScore) + " }\n";
		counter++;
	}

	testOut += "UserCode = " + to_string(ProblemFK) + "\n";
	OutputLog(testOut.c_str());
	// FIXME Test Code End -- Yoto

	char dir_name[MAXPATHLEN + 16], input_filename[MAXPATHLEN + 16];
	char buffer[MAXPATHLEN * 2 + 16];
	puts("judge");

	sprintf(dir_name, "%s/%d", SystemConf.DataDir.c_str(), ProblemFK);
	DIR *dp = opendir(dir_name);
	if (dp == NULL) {
		ErrorCode = SOLUTION_SYSTEM_ERROR;
		LastState = "No data files";
		throw "Can't open data dir";
	}
	std::vector<std::string> in_files;
	struct dirent *ep;
	ep = readdir(dp);
	while (ep) {
		int len = strlen(ep->d_name);
		if (len > 3 && 0 == strcasecmp(ep->d_name + len - 3, ".in")) {
			in_files.push_back(std::string(ep->d_name));
		}
		ep = readdir(dp);
	}
	closedir(dp);
	if (in_files.empty()) {
		ErrorCode = SOLUTION_SYSTEM_ERROR;
		LastState = "No data files";
		throw "Data folder is empty";
	}
	std::sort(in_files.begin(), in_files.end());

	int total_score = 0, total_time = 0, max_memory = 0, dir_len = strlen(dir_name);
	const int full_score = 100;
	int case_score = full_score / in_files.size();
	if (case_score <= 0)
		case_score = 1;

	int status;
	std::string tips;
	for (std::string &d_name : in_files) {
		path data_directory(dir_name), filename(d_name),
				inputFile, tempFile, outputFile, ansFile;

		inputFile = data_directory / d_name;
		outputFile = "user.out";
		ansFile = inputFile;
		ansFile.replace_extension("out");

		if (SystemConf.TempDir.size() > 0) {
			path temp_directory(SystemConf.TempDir);

			try {
				tempFile = temp_directory / d_name;

				if (exists(tempFile)) {
					remove(tempFile);
				}

				copy_file(inputFile, tempFile);

				inputFile = tempFile;
				outputFile = temp_directory / outputFile;
				OutputLog("Temp file created.");
			}
			catch (filesystem_error &ex) {
				OutputLog("Failed to create temp file.", ex.what());
			}

		}

		ExecutionInfo result;
		int get_score = case_score;

		if (run_judge(TargetPath, inputFile.string().c_str(), outputFile.string().c_str(), TimeLimit,
		              (SystemConf.FindLanguage(LanguageType)->ExtraMemory + MemoryLimit) << 10 /*to byte*/, &result)) {
			ErrorCode = SOLUTION_SYSTEM_ERROR;
			LastState = "Cannot run target program";
			throw "Cannot run target program";
		} else if (result.State == 0) {

			FILE *fanswer = fopen(ansFile.string().c_str(), "rb");
			if (fanswer) {
				FILE *foutput = fopen(outputFile.string().c_str(), "rb"), *finput;
				if (foutput) {
					ValidatorInfo info;

					switch (ComparisonMode >> 16) {
						case CMP_tra:
#ifdef USE_CENA_VALIDATOR
							info = validator_cena(fanswer, foutput);
#else
							info = validator(fanswer, foutput);//traditional OI comparison (Ignore trailing space)
#endif
							break;
						case CMP_float:
							info = validator_float(fanswer, foutput, (ComparisonMode & 0xffff)); //precision comparison
							break;
						case CMP_int:
							info = validator_int(fanswer, foutput);
							break;
						case CMP_spj:
							sprintf(input_filename, "%s/%s", dir_name, d_name.c_str());
							info = run_spj(buffer, input_filename, &get_score, dir_name);//in call_ruc.cpp
							break;
						default:
							info.Result = -1; //validator error
					}
					fclose(foutput);

					int s = info.Result;
					if (!s) {
						status = SOLUTION_ACCEPT;
						tips = "Good Job!";
						total_score += get_score;
					} else if (s == -1) {
						status = SOLUTION_VALIDATOR_ERROR;
						tips = "Please contact administrator.";
						get_score = 0;
					} else if (s == 4) { // for spj
						status = (get_score == case_score) ? SOLUTION_ACCEPT : SOLUTION_WRONG_ANSWER;
						total_score += get_score;
						tips = info.UserMismatch;
						free(info.UserMismatch);
					} else {
						status = SOLUTION_WRONG_ANSWER;
						get_score = 0;
						if (s == 1) {
							tips = "Output mismatch.\nYours: ";
							tips += info.UserMismatch;
							tips += "\nAnswer: ";
							tips += info.StandardMismatch;
							free(info.UserMismatch);
							free(info.StandardMismatch);
						} else if (s == 2)
							tips = "Your output is longer than standard output.";
						else if (s == 3)
							tips = "Your output is shorter than standard output.";
						else //unknown result
							tips = "";
					}
				} else {
					status = SOLUTION_WRONG_ANSWER;
					get_score = 0;
					tips = "Cannot find output file.";
				}
				fclose(fanswer);
			} else {
				OutputLog((std::string("Info: No answer file ") + buffer).c_str());
				get_score = 0;
				tips = "No answer file";
				status = SOLUTION_WRONG_ANSWER;
			}
		} else { //RE,TLE,MLE
			get_score = 0;
			status = result.State;
			if (status == SOLUTION_RUNTIME_ERROR)
				tips = result.Info;
			else
				tips = "";
		}
		total_time += result.Time;
		if (result.Memory > max_memory)
			max_memory = result.Memory;

		printf("status %d %s\n", status, tips.c_str());
		if (ErrorCode == -1 && status != SOLUTION_ACCEPT) {//only store the first error infomation
			ErrorCode = status;
			LastState = tips;
		}

		if (!tempFile.empty() && exists(tempFile)) {
			remove(tempFile);
		}

		if (exists(outputFile)) {
			remove(outputFile);
		}

		std::unique_lock<std::mutex> Lock(*(std::mutex *) QueryMutex);
		TestCaseDetail.push_back({status, result.Time, result.Memory, tips, get_score});
	}

	if (ErrorCode == -1) //No error
		ErrorCode = SOLUTION_ACCEPT;

	//use score,mem_limit,time_limit to store result
	SolutionScore = total_score / double(case_score * in_files.size()) * full_score + 0.500001;
	MemoryLimit = max_memory;
	TimeLimit = total_time;
	printf("error_code %d, time %dms, memory %dkB, score %d\n", ErrorCode, TimeLimit, MemoryLimit, SolutionScore);
}

void solution::WriteDatabase() throw(const char *) {
	int id = get_next_solution_id();
	printf("solution_id: %d\n", id);
	if (this->UserName.size() == 0)
		throw "User id is empty";
	write_result_to_database(id, this);
}
