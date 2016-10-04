#include <string>
#include <vector>
#include <ctime>
#include <mutex>

class SingleTestCaseResult {
public:
	int ErrorCode, TimeLimit, MemoryLimit;
	std::string ResultDetail;
	int CaseScore;
};

class Solution {
public:
	int ProblemFK, ComparisonMode, LanguageType, TimeLimit, MemoryLimit, Score, ErrorCode;
	bool IsCodeOpenSource;
	unsigned char SolutionType;
	std::string UserCode, UserName, Key, LastState; // FIXME IDKWIM Key
	std::vector<SingleTestCaseResult> TestCaseResults;
	std::mutex *QueryMutex;//use void* to avoid including <mutex> (Oh fuck you why? -- Yoto)
	time_t TimeStamp;
//	The following code was never used
//#ifdef DUMP_FOR_DEBUG
//	std::string raw_post_data;
//#endif
	std::string TargetPath; // FIXME IDKWIM // Fuck you! you knew std::string, aren't you? Why use const char (*)? I've fixed this for you. -- Yoto

	Solution();
	~Solution();

	void ClonePropertiesFrom(const Solution &) throw();
	bool CompileUserCode() throw(const char *);
	void JudgeSolution() throw(const char *);
	void WriteResultToDB() throw(const char *);
};

class ExecutionStatus {
public:
	int State, Time, Memory;
	std::string *AdditionalInfo;
};

struct ValidatorResult {
	int ResultCode; // FIXME IDKWIM
	char *UserMismatch;
	char *StandardMismatch; // FIXME IDKWIM
};

enum {
	TYPE_normal = 0, TYPE_rejudge = 1
};
enum {
	MSG_problem,
	MSG_lang,
	MSG_time,
	MSG_mem,
	MSG_score,
	MSG_code,
	MSG_user,
	MSG_key,
	MSG_share,
	MSG_compare,
	MSG_rejudge
};
enum {
	RES_AC = 0, RES_CE = 7, RES_TLE = 2, RES_MLE = 3, RES_WA = 4, RES_RE = 5, RES_VE = 99, RES_SE = 100
};

typedef int (*run_compiler_def)(const char *, char *, int);

typedef int (*run_judge_def)(const char *, const char *, const char *, int, int, ExecutionStatus *);

std::string getTargetPath();

void applog(const char *str, const char *info = "") throw();

bool read_config();

bool start_http_interface();

bool init_mysql_con() throw();

char *JUDGE_get_progress(const char *);

char *JUDGE_accept_submit(Solution *&sol);

char *JUDGE_start_rejudge(Solution *&sol);

bool clean_files() throw();

int get_next_solution_id() throw(const char *);

void write_result_to_database(int, Solution *) throw(const char *);

void get_exist_solution_info(int solution_id, Solution *sol) throw(const char *);

void update_exist_solution_info(int solution_id, Solution *sol) throw(const char *);

void update_problem_rejudged_status(int problem_id) throw(const char *);

void refresh_users_problem(int problem_id) throw(const char *);

void get_solution_list(std::vector<int> &rejudge_list, int problem_id) throw(const char *);

struct ValidatorResult validator_cena(FILE *fstd, FILE *fuser);

extern "C" {
struct ValidatorResult validator(FILE *fstd, FILE *fuser);
struct ValidatorResult validator_int(FILE *fstd, FILE *fuser);
struct ValidatorResult validator_float(FILE *fstd, FILE *fuser, int);
}

int run_judge(std::string, const char *, const char *, int, int, ExecutionStatus *);

ValidatorResult run_spj(char *datafile_out, char *datafile_in, int *score, char *data_dir);


