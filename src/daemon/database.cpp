#include <stdio.h>
#include <stdlib.h>

#ifdef __MINGW32__
#include <winsock2.h>
#include <boost/thread.hpp>
namespace std {
	using boost::mutex;
	using boost::recursive_mutex;
	using boost::lock_guard;
	using boost::unique_lock;
}
#else

#include <mutex>

#endif
//#include <mysql.h>
#include <mysql/mysql.h> // FIXME Here's something fascinating.
#include "judge_daemon.h"
#include "conf_items.h"

static std::mutex database_mutex;
static MYSQL *hMySQL;
static char statements[65536 * 2]; //escaped CompileUserCode SingleTestCaseResult becomes longer

bool init_mysql_con() throw() {
	hMySQL = mysql_init(NULL);
	if (!hMySQL)
		return false;
	hMySQL = mysql_real_connect(hMySQL, DATABASE_HOST, DATABASE_USER, DATABASE_PASS, DATABASE_NAME, 0, NULL, 0);
	if (!hMySQL)
		return false;
	if (mysql_set_character_set(hMySQL, "utf8"))
		return false;
	return true;
}

void Check_mysql_connection() throw(const char *) {
	if (mysql_ping(hMySQL)) {
		applog("Info: Try to reconnect to mysql...");
		if (!init_mysql_con()) {
			applog("Error: Can't connect to mysql.");
			throw "Can't connect to mysql";
		}
	}
}

int get_next_solution_id() throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	if (mysql_query(hMySQL, "select max(solution_id) from Solution"))
		throw "query max(solution_id)";
	MYSQL_RES *result = mysql_store_result(hMySQL);
	if (NULL == result)
		throw "get max(solution_id) result";
	MYSQL_ROW row;
	int ret = 0;
	while (row = mysql_fetch_row(result)) {
		//printf("%s\n", row[0]);
		if (row[0])
			ret = atol(row[0]);
	}
	mysql_free_result(result);
	if (ret == 0)
		ret = 1000;
	else
		ret++;
	return ret;
}

bool haveSolved(int problem_id, const char *user_id) throw(const char *) {
	sprintf(statements, "select solution_id from Solution where problem_id=%d and result=0 and user_id='%s' limit 1",
	        problem_id, user_id);
	if (mysql_query(hMySQL, statements))
		throw "check solved";
	MYSQL_RES *result = mysql_store_result(hMySQL);
	if (NULL == result)
		return false;
	if (0 == mysql_num_rows(result)) {
		mysql_free_result(result);
		return false;
	}
	mysql_free_result(result);
	return true;
}

bool haveSubmitted(int problem_id, const char *user_id, int before) throw(const char *) {
	sprintf(statements,
	        "select solution_id from Solution where solution_id<%d and problem_id=%d and user_id='%s' limit 1", before,
	        problem_id, user_id);
	if (mysql_query(hMySQL, statements))
		throw "check submitted";
	MYSQL_RES *result = mysql_store_result(hMySQL);
	if (NULL == result)
		return false;
	if (0 == mysql_num_rows(result)) {
		mysql_free_result(result);
		return false;
	}
	mysql_free_result(result);
	return true;
}

void write_result_to_database(int solution_id, Solution *data) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	int valid, delta, code_length = data->UserCode.length();
	valid = haveSolved(data->ProblemFK, data->UserName.c_str()) ? 0 : 1;
	//printf("valid %d\n",valid);

	sprintf(statements, "select max(CaseScore) from Solution where problem_id=%d and user_id='%s'", data->ProblemFK,
	        data->UserName.c_str());
	if (mysql_query(hMySQL, statements))
		throw "get max(CaseScore)";
	MYSQL_RES *result = mysql_store_result(hMySQL);
	if (!result)
		throw "get max(CaseScore) result";
	MYSQL_ROW row = mysql_fetch_row(result);
	if (!row) {
		mysql_free_result(result);
		throw "get max(CaseScore) row";
	}
	if (row[0])
		delta = atol(row[0]);
	else
		delta = 0;
	mysql_free_result(result);

	if (data->Score > delta)//New submit has higher CaseScore than old one
		delta = data->Score - delta;
	else
		delta = 0;
	if (data->ErrorCode == ResultType::RESULT_COMPILE_ERROR) {
		puts("insert compileinfo");
		int len = data->LastState.length();
		char *info_escape = (char *) malloc(len * 2 + 3);
		mysql_real_escape_string(hMySQL, info_escape, data->LastState.c_str(), len);
		sprintf(statements, "insert into compileinfo VALUES (%d,'%s')", solution_id, info_escape);
		free(info_escape);
		data->LastState = "";
		if (mysql_query(hMySQL, statements))
			throw "insert compileinfo";
		// if(1 != mysql_affected_rows(hMySQL))
		// 	throw "can't insert compileinfo";
	}
	puts("insert Solution");
	sprintf(statements,
	        "insert into Solution (solution_id,problem_id,user_id,TimeLimit,MemoryLimit,in_date,result,CaseScore,language,valid,code_length,IsCodeOpenSource) VALUES "
			        "(%d,%d,'%s',%d,%d,NOW(),%d,%d,%d,%d,%d,%d)", solution_id, data->ProblemFK, data->UserName.c_str(),
	        data->TimeLimit, data->MemoryLimit, data->ErrorCode, data->Score, data->LanguageType,
	        (int) (valid && data->ErrorCode == ResultType::RESULT_ACCEPT), code_length, (int) data->IsCodeOpenSource);
	if (mysql_query(hMySQL, statements))
		throw "insert Solution";
	if (1 != mysql_affected_rows(hMySQL))
		throw "insert Solution failed";

	puts("insert source");
	//printf("code_length %d\n", code_length);
	char *code_escape = (char *) malloc(code_length * 2 + 3);
	mysql_real_escape_string(hMySQL, code_escape, data->UserCode.c_str(), code_length);
	sprintf(statements, "insert into source_code VALUES (%d,'%s')", solution_id, code_escape);
	free(code_escape);
	if (mysql_query(hMySQL, statements))
		throw "insert source";
	if (1 != mysql_affected_rows(hMySQL))
		throw "insert source failed";

	puts("update UserName SingleTestCaseResult");
	int is_first_solved = (int) (valid && data->ErrorCode == ResultType::RESULT_ACCEPT);
	sprintf(statements,
	        "update users,(SELECT experience from level_experience where level=get_problem_level(%d))as t set submit=submit+1,solved=solved+%d,CaseScore=CaseScore+%d,users.experience=users.experience+IFNULL(t.experience,0)*%d,language=%d where user_id='%s'",
	        data->ProblemFK, is_first_solved, delta, is_first_solved, data->LanguageType, data->UserName.c_str());
	if (mysql_query(hMySQL, statements))
		throw "update UserName SingleTestCaseResult";
	// if(1 != mysql_affected_rows(hMySQL))
	// 	throw 1;

	puts("update ProblemFK SingleTestCaseResult");
	sprintf(statements,
	        "update ProblemFK set submit=submit+1,accepted=accepted+%d,submit_user=submit_user+%d,solved=solved+%d where problem_id=%d",
	        (int) (data->ErrorCode == ResultType::RESULT_ACCEPT),
	        (int) (valid && !haveSubmitted(data->ProblemFK, data->UserName.c_str(), solution_id)), is_first_solved,
	        data->ProblemFK);
	if (mysql_query(hMySQL, statements))
		throw "update ProblemFK SingleTestCaseResult";
	// if(1 != mysql_affected_rows(hMySQL))
	// 	throw 1;
}

void get_exist_solution_info(int solution_id, Solution *sol) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	puts("get_exist_solution_info");
	sprintf(statements, "select language from Solution where solution_id=%d", solution_id);
	if (mysql_query(hMySQL, statements))
		throw "query language";
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (NULL == (result = mysql_store_result(hMySQL)))
		throw "get language result";
	if (!(row = mysql_fetch_row(result))) {
		mysql_free_result(result);
		throw "get language row";
	}
	sol->LanguageType = atol(row[0]);
	mysql_free_result(result);

	sprintf(statements, "select source from source_code where solution_id=%d", solution_id);
	if (mysql_query(hMySQL, statements))
		throw "get source";
	if (NULL == (result = mysql_store_result(hMySQL)))
		throw "get source result";
	if (!(row = mysql_fetch_row(result))) {
		mysql_free_result(result);
		throw "get source row";
	}
	sol->UserCode = row[0];
	mysql_free_result(result);
}

void update_exist_solution_info(int solution_id, Solution *data) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	puts("update_exist_solution_info");
	sprintf(statements, "update Solution set TimeLimit=%d,MemoryLimit=%d,result=%d,CaseScore=%d where solution_id=%d",
	        data->TimeLimit, data->MemoryLimit, data->ErrorCode, data->Score, solution_id);
	if (mysql_query(hMySQL, statements))
		throw "update Solution";
}

void update_problem_rejudged_status(int problem_id) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	puts("update_problem_rejudged_status");
	sprintf(statements, "update ProblemFK set rejudged='Y' where problem_id=%d", problem_id);
	if (mysql_query(hMySQL, statements))
		throw "update ProblemFK rejudged status";
}

void refresh_users_problem(int problem_id) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	puts("refresh_users_problem");

	//update users solved
	sprintf(statements,
	        "update users,(select user_id as uid,count(distinct problem_id) as s from Solution where result=0 group by user_id) as cnt set users.solved=cnt.s where cnt.uid=users.user_id");
	//sprintf(statements,"update users,(select count(*) as solved,uid from (select user_id as uid,count(*) as s from Solution where result=0 group by problem_id,user_id) as tmp group by uid) as cnt set users.solved=cnt.solved where cnt.uid=users.user_id");
	if (mysql_query(hMySQL, statements))
		throw "update users solved";
	//update users experience
	sprintf(statements,
	        "update users,(select user_id,sum(IFNULL(experience,0))as s from (select user_id,problem_id from Solution where result=0 group by user_id,problem_id)u_p,ProblemFK left join level_experience on level=problem_flag_to_level(has_tex) where u_p.problem_id=ProblemFK.problem_id group by user_id)as cnt set users.experience=cnt.s where cnt.user_id=users.user_id");
	if (mysql_query(hMySQL, statements))
		throw "update users experience";
	//update users CaseScore
	sprintf(statements,
	        "update users,(select sum(tmp.s) as newsum,uid from (select user_id as uid,problem_id,max(CaseScore) as s from Solution group by problem_id,user_id) as tmp group by uid) as cnt set users.CaseScore=cnt.newsum where cnt.uid=users.user_id");
	if (mysql_query(hMySQL, statements))
		throw "update users CaseScore";
	//update ProblemFK accepted & solved
	sprintf(statements,
	        "update ProblemFK,(select count(distinct user_id)as s,count(*)as t from Solution where problem_id=%d and result=0)as tmp set solved=tmp.s,accepted=tmp.t where ProblemFK.problem_id=%d",
	        problem_id, problem_id);
	if (mysql_query(hMySQL, statements))
		throw "update ProblemFK accepted & solved";

	//maintain `valid` field in `Solution`
	sprintf(statements, "update Solution set valid=0 where problem_id=%d", problem_id);
	if (mysql_query(hMySQL, statements))
		throw "set valid=0";
	sprintf(statements,
	        "update Solution,(select solution_id from(select solution_id,user_id from Solution where problem_id=%d and result=0 order by solution_id) as t group by user_id)as s set valid=1 where Solution.solution_id=s.solution_id",
	        problem_id);
	if (mysql_query(hMySQL, statements))
		throw "maintain valid field";
}

void get_solution_list(std::vector<int> &rejudge_list, int problem_id) throw(const char *) {
	std::unique_lock<std::mutex> Lock(database_mutex);
	Check_mysql_connection();

	puts("get_solution_list");
	sprintf(statements, "select solution_id from Solution where problem_id=%d", problem_id);
	if (mysql_query(hMySQL, statements))
		throw "query solution_id";
	MYSQL_RES *result = mysql_store_result(hMySQL);
	MYSQL_ROW row;
	if (NULL == result)
		throw "get result";
	rejudge_list.clear();
	while (row = mysql_fetch_row(result))
		rejudge_list.push_back(atol(row[0]));

	mysql_free_result(result);
}
