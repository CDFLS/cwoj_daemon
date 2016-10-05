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
	std::mutex *QueryMutex; //use void* to avoid including <mutex> (Oh fuck you why? -- Yoto)
	time_t TimeStamp;
	std::string RawPostData; // This should never be used.
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

enum JudgeType {
	JT_NORMAL = 0, JT_REJUDGE = 1
};
enum HttpMessageType {
	MSG_PROBLEM,
	MSG_LANGUAGE,
	MSG_TIME,
	MSG_MEMORY,
	MSG_SCORE,
	MSG_CODE,
	MSG_USER,
	MSG_KEY, // FIXME IDKWIM
	MSG_SHARE,
	MSG_COMPARE,
	MSG_REJUDGE
};
enum ResultType {
	RESULT_ACCEPT = 0,
	RESULT_COMPILE_ERROR = 7,
	RESULT_TIME_LIMIT_EXCEEDED = 2, // FIXME No usage
	RESULT_MEMORY_LIMIT_EXCEEDED = 3, // FIXME No usage
	RESULT_WRONG_ANSWER = 4,
	RESULT_RUNTIME_ERROR = 5,
	RESULT_VALIDATOR_ERROR = 99,
	RESULT_SYSTEM_ERROR = 100
};

typedef int (*run_compiler_def)(const char *, char *, int);

typedef int (*run_judge_def)(const char *, const char *, const char *, int, int, ExecutionStatus *);

std::string getTargetPath();

void applog(const char *str, const char *info = "") throw();

void applog(std::string tag, std::string content = "") throw();

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


