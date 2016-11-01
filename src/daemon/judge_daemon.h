#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <mutex>

enum JudgeActionType {
    JUDGE_ACTION_NORMAL = 0, JUDGE_ACTION_REJUDGE = 1
};

enum HttpMessageType {
    MESSAGE_PROBLEM,
    MESSAGE_LANGUAGE,
    MESSAGE_TIME,
    MESSAGE_MEMORY,
    MESSAGE_SCORE,
    MESSAGE_CODE,
    MESSAGE_USER,
    MESSAGE_KEY,
    MESSAGE_SHARE,
    MESSAGE_COMPARE,
    MESSAGE_REJUDGE
};

enum JudgeResultType {
    SOLUTION_ACCEPT = 0,
    SOLUTION_COMPILATION_ERROR = 7,
    SOLUTION_TIME_LIMIT_EXCEEDED = 2,
    SOLUTION_MEMORY_LIMIT_EXCEEDED = 3,
    SOLUTION_WRONG_ANSWER = 4,
    SOLUTION_RUNTIME_ERROR = 5,
    SOLUTION_VALIDATOR_ERROR = 99,
    SOLUTION_SYSTEM_ERROR = 100
};

enum ValidatorResult {
    VALIDATOR_FUCKED = -1,
    VALIDATOR_IDENTICAL = 0,
    VALIDATOR_MISMATCH = 1,
    VALIDATOR_LONGER = 2,
    VALIDATOR_SHORTER = 3
};

class SingleTestCaseReport {
public:
    int ErrorCode, TimeProfile, MemoryProfile;
    std::string AdditionalInformation;
    int CaseScore;
};

class ExecutionInfo {
public:
    int State, Time, Memory;
    char *Info;
};

class ValidatorInfo {
public:
    int Result;
    char *UserMismatch;
    char *StandardMismatch;
};

class solution {
public:
    int ProblemFK, ComparisonMode, LanguageType, TimeLimit, MemoryLimit, SolutionScore, ErrorCode;
    bool IsCodeOpenSourced;
    unsigned char SolutionType;
    std::string SourceCode, UserName, Key, LastState;
    std::vector<SingleTestCaseReport> TestCaseDetail;
    std::mutex *QueryMutex;//use void* to avoid including <mutex>
    time_t TimeStamp;
    const char *TargetPath;
#ifdef DUMP_FOR_DEBUG
    std::string RawPostData;
#endif

    solution();

    ~solution();

    void CloneFrom(const solution &) throw();

    bool Compile() throw(const char *);

    void Judge() throw(const char *);

    void WriteDatabase() throw(const char *);
};

typedef int (*run_compiler_def)(const char *, char *, int);

typedef int (*run_judge_def)(const char *, const char *, const char *, int, int, ExecutionInfo *);

const char *getTargetPath();

void OutputLog(const char *str, const char *info = "") throw();

void OutputLog(std::string str, std::string info = "") throw();

bool ReadConfigurationFile();

bool StartHttpInterface();

bool InitMySQLConnection() throw();

char *JUDGE_get_progress(const char *);

char *JUDGE_accept_submit(solution *&sol);

char *JUDGE_start_rejudge(solution *&sol);

bool clean_files() throw();

int get_next_solution_id() throw(const char *);

void write_result_to_database(int, solution *) throw(const char *);

void get_exist_solution_info(int solution_id, solution *sol) throw(const char *);

void update_exist_solution_info(int solution_id, solution *sol) throw(const char *);

void update_problem_rejudged_status(int problem_id) throw(const char *);

void refresh_users_problem(int problem_id) throw(const char *);

void get_solution_list(std::vector<int> &rejudge_list, int problem_id) throw(const char *);

struct ValidatorInfo validator_cena(FILE *fstd, FILE *fuser);

extern "C" {
struct ValidatorInfo validator(FILE *fstd, FILE *fuser);
struct ValidatorInfo validator_int(FILE *fstd, FILE *fuser);
struct ValidatorInfo validator_float(FILE *fstd, FILE *fuser, int);
}

ValidatorInfo run_spj(char *datafile_out, char *datafile_in, int *score, char *data_dir);

