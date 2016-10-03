#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <queue>

#ifdef __MINGW32__
#include <windows.h>

//from: http://kbokonseriousstuff.blogspot.com/2011/08/c11-standard-library-threads-with-mingw.html
#include <boost/thread.hpp>
namespace std {
	using boost::mutex;
	using boost::recursive_mutex;
	using boost::lock_guard;
	using boost::condition_variable;
	using boost::unique_lock;
	using boost::thread;
}

namespace std {
	using boost::to_string;
}
#else

#include <mutex>
#include <thread>
#include <condition_variable>

#endif

#include "judge_daemon.h"

static std::map<std::string, Solution *> finder;
static std::queue<Solution *> waiting, removing;
static std::mutex waiting_mutex, removing_mutex, finder_mutex, applog_mutex, rejudge_mutex;
static std::condition_variable notifier, rejudge_notifier;
static volatile bool rejudging, filled;
static std::vector<int> rejudge_list;
static Solution rejudge_init;

const float build_version = 1.03;

#ifdef USE_DLL_ON_WINDOWS
run_compiler_def run_compiler;
run_judge_def run_judge;
#endif

static std::string target_path;

std::string getTargetPath() {
	return target_path;
}

void applog(const char *str, const char *info) throw() {
	time_t now_time = time(NULL);
	std::unique_lock<std::mutex> Lock(applog_mutex);
	char time_str[24];
#ifdef __MINGW32__
	std::strftime(time_str, 24, "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
#else
	std::strftime(time_str, 24, "%F %T", std::localtime(&now_time));
#endif
	printf("[%s] %s (%s)\n", time_str, str, info);
	fflush(stdout);
}

void json_builder(std::ostringstream &, Solution *);

char *JUDGE_get_progress(const char *query) {
	try {
		Solution *target;
		do {
			std::unique_lock<std::mutex> Lock_map(finder_mutex);
			auto it = finder.find(std::string(query + 7));//query must start with "query_",checked in http.cpp
			if (finder.end() == it) {
				char *response = (char *) malloc(21);
				if (response)
					strcpy(response, "{\"State\":\"invalid\"}");
				return response;
			}
			target = it->second;
		} while (0);

		std::ostringstream json;
		do {
			std::unique_lock<std::mutex> Lock_map(*(std::mutex *) (target->QueryMutex));
			json_builder(json, target);
		} while (0);

		const std::string &buf = json.str();
		char *response = (char *) malloc(buf.size() + 1);
		if (response)
			strcpy(response, buf.c_str());
		return response;
	} catch (...) {
		applog("Info: Exception when query");
	}
	char *response = (char *) malloc(21);
	if (response)
		strcpy(response, "{\"State\":\"invalid\"}");
	return response;
}

void thread_remove() {
	for (;;) {
		std::unique_lock<std::mutex> Lock(removing_mutex);
		if (removing.empty()) {
			//puts("empty");
			Lock.unlock();
#ifdef _WIN32
			Sleep(20000);
#else
			sleep(20);
#endif
			//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			continue;
		}
		time_t now = time(0);
		if ((now - removing.front()->TimeStamp) < 25) {
			//printf("%d - %d < 2\n", now, removing.front()->TimeStamp);
			Lock.unlock();
			int remain = (now - removing.front()->TimeStamp) + 1;
#ifdef _WIN32
			Sleep(remain*1000);
#else
			sleep(remain);
#endif
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}
		puts("Information deleted");
		Solution *cur = removing.front();
		finder.erase(cur->Key);
		removing.pop();
#ifdef _WIN32
		Sleep(1500);
#else
		sleep(1);
#endif
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		delete cur;
	}
}

void thread_judge() {
	for (;;) {
		Solution *cur;
		do {
			std::unique_lock<std::mutex> Lock(waiting_mutex);
			notifier.wait(Lock, []() -> bool { return !waiting.empty(); });
			cur = waiting.front();
			waiting.pop();
		} while (0);

		try {
			if (!clean_files()) {
				applog("Info: Cannot clean working directory.");
			}
			if (cur->CompileUserCode())
				cur->JudgeSolution();
			if (cur->SolutionType == TYPE_normal) {
				cur->WriteResultToDB();
			}
			cur->TimeStamp = time(0);
		} catch (const char *e) {
			std::string message("Error: Exception occur when judging ");
			message += cur->Key;
			applog(message.c_str(), e);

			try {
				std::unique_lock<std::mutex> Lock_map(*(std::mutex *) (cur->QueryMutex));
				cur->TestCaseResults.clear();
				cur->TestCaseResults.push_back({RES_SE, 0, 0, cur->LastState, 0});
				cur->TimeStamp = time(0);

#ifdef DUMP_FOR_DEBUG
				message = "../dump/dump_";
				message += cur->key;
				std::ofstream dump_f(message.c_str());
				if(dump_f.is_open()) {
					dump_f << cur->raw_post_data;
					dump_f.close();
					puts("Dump file created");
				}
#endif
			} catch (...) {
				applog("Info: Can't create dump file.");
			}
		}

		if (cur->SolutionType == TYPE_normal) {
			std::unique_lock<std::mutex> Lock(removing_mutex);
			removing.push(cur);
		} else if (cur->SolutionType == TYPE_rejudge) {
			rejudging = false;
		}
	}
}

void thread_rejudge() {
	std::unique_lock<std::mutex> Lock(rejudge_mutex);
	for (;;) {
		rejudge_notifier.wait(Lock, []() -> bool { return filled; });
		filled = false;

		puts("rejudge thread running");
		for (int solution_id : rejudge_list) {
			printf("rejudging %d\n", solution_id);
			Solution *sol = new Solution;
			if (!sol) {
				applog("Error: Can't allocate MemoryLimit, stop rejudging");
				break;
			}
			try {
				sol->ClonePropertiesFrom(rejudge_init); //Get ProblemFK settings, like ComparisonMode
				get_exist_solution_info(solution_id, sol);
				sol->ErrorCode = -1;
				sol->Key = std::to_string(solution_id);

				rejudging = true;
				do { //insert to queue
					std::unique_lock<std::mutex> Lock(waiting_mutex);
					waiting.push(sol);
					notifier.notify_one();
				} while (0);

				//wait until finished
				while (rejudging);

				update_exist_solution_info(solution_id, sol);
			} catch (const char *e) {
				char tmp[64];
				sprintf(tmp, "Error: Exception occur when rejudging %d", solution_id);
				applog(tmp, e);
			}
			delete sol;
		}
		try {
			update_problem_rejudged_status(rejudge_init.ProblemFK);
			refresh_users_problem(rejudge_init.ProblemFK);
		} catch (const char *e) {
			if (e != "maintain valid field")
				applog("Error: Exception occur when refreshing UserName SingleTestCaseResult", e);
		}
		applog("Info: Rejudge thread finished");
	}
}

char *JUDGE_start_rejudge(Solution *&new_sol) {
	char *p = (char *) malloc(8);
	if (rejudge_mutex.try_lock()) {
		try {
			rejudge_init = *new_sol;
			get_solution_list(rejudge_list, rejudge_init.ProblemFK);

			filled = true;
			rejudge_notifier.notify_one();
			rejudge_mutex.unlock();
			strcpy(p, "OK");
			return p;
		} catch (const char *e) {
			applog("Error: Cannot start rejudging", e);
			rejudge_mutex.unlock();

			strcpy(p, "error");
			return p;
		}
	}
	strcpy(p, "another");
	return p;
}

char *JUDGE_accept_submit(Solution *&new_sol) {
	try {
		if (!new_sol)
			throw "invalid pointer";
		new_sol->ErrorCode = -1;
		new_sol->TimeStamp = 0;
		do {
			std::unique_lock<std::mutex> Lock(waiting_mutex);
			waiting.push(new_sol);
			notifier.notify_one();
		} while (0);
		do {
			std::unique_lock<std::mutex> Lock(finder_mutex);
			finder[new_sol->Key] = new_sol;
		} while (0);
	} catch (...) {
		applog("Error: Exception in JUDGE_accept_submit()");
		char *p = (char *) malloc(6);
		strcpy(p, "error");
		return p;
	}
	new_sol = NULL;//avoid being freed
	char *p = (char *) malloc(3);
	strcpy(p, "OK");
	return p;
}

//TODO: watchdog
int main(int argc, char **argv) {
	printf("CWOJ Judging Service ver %.2f started.\n", build_version);
	//enter program directory to read ini files
#if defined(_WIN32) || defined(__linux__)
#ifdef _WIN32
	int size = GetModuleFileNameA(NULL, TargetPath, MAXPATHLEN);
	if(size <= 0) {
		applog("Error: Cannot get program directory, Exit...");
		exit(1);
	}
	for(int i=size-1; i>=0; i--)
		if(TargetPath[i] == '\\') {
			TargetPath[i+1] = '\0';
			break;
		}
	printf("entering %s\n", TargetPath);
	if(!SetCurrentDirectory(TargetPath)) {
		applog("Error: Cannot enter program directory, Exit...");
		exit(1);
	}
#else
	int size = readlink("/proc/self/exe", (char *) target_path.c_str(), MAXPATHLEN);
	if (size <= 0) {
		applog("Error: Cannot get program directory, Exit...");
		exit(1);
	}
	target_path[size] = '\0';
	for (int i = size - 1; i >= 0; i--)
		if (target_path[i] == '/') {
			target_path[i + 1] = '\0';
			break;
		}
	printf("entering %s\n", target_path.c_str());
	if (0 != chdir(target_path.c_str())) {
		applog("Error: Cannot enter program directory, Exit...");
		exit(1);
	}
#endif
#else
	// I don't know
#endif

	if (!read_config()) {
		applog("Error: Cannot read and parse the config.ini, Exit...");
		exit(1);
	}
	if (!init_mysql_con()) {
		applog("Error: Cannot connect to mysql, Exit...");
		exit(1);
	}
#ifdef __MINGW32__
	mkdir("temp");
#else
	mkdir("temp", 0777);
#endif
	if (0 != chdir("temp")) {
		applog("Error: Cannot enter working directory, Exit...");
		exit(1);
	}
#ifndef __MINGW32__ //used when run program on *nix only
	if (NULL == getcwd((char *) target_path.c_str(), MAXPATHLEN)) {
		applog("Error: Cannot get working directory, Exit...");
		exit(1);
	}
	strcat((char *) target_path.c_str(), "/target.exe");
	printf("target: %s\n", target_path.c_str());
#else
	strcpy(TargetPath,"target.exe");
#endif

	std::thread thread_j(thread_judge);
	std::thread thread_r(thread_remove);
	std::thread thread_re(thread_rejudge);

	if (!start_http_interface()) {
		applog("Error: Cannot open http interface, Exit...");
		exit(1);
	}
	applog("Started successfully.Waiting for submitting...");
	thread_j.join();
}
