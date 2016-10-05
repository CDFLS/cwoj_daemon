#include <string>
#include <sstream>
#include "judge_daemon.h"

class encoder {
	char *buf;
public:
	encoder(std::string &in) {
		buf = (char *) malloc((in.size() << 1) + 3);
		if (!buf)
			throw "encoder: Can't allocate MemoryLimit";
		char *p = buf;
		*(p++) = '"';
		for (char &c : in) {
			switch (c) {
				case '"':
					*(p++) = '\\';
					*(p++) = '"';
					break;
				case '\\':
					*(p++) = '\\';
					*(p++) = '\\';
					break;
				case '\n':
					*(p++) = '\\';
					*(p++) = 'n';
					break;
				case '\r':
					*(p++) = '\\';
					*(p++) = 'r';
					break;
				default:
					*(p++) = c;
			}
		}
		*(p++) = '"';
		*p = 0;
	}

	~encoder() {
		free(buf);
	}

	friend std::ostream &operator<<(std::ostream &os, const encoder &me) {
		os << me.buf;
		return os;
	}
};

void json_builder(std::ostringstream &json, Solution *target) {
	if (target->TimeStamp != 0)
		json << "{\"State\":\"finish\",\"detail\":[";
	else
		json << "{\"State\":\"wait\",\"detail\":[";
	/*if(target->ErrorCode == ResultType::RESULT_COMPILE_ERROR){
		json<<ResultType::RESULT_COMPILE_ERROR<<",0,0,"<<encoder(target->LastState)<<"]}";
		return;
	}*/
	for (SingleTestCaseResult &c : target->TestCaseResults)
		json << "[" << c.ErrorCode << ',' << c.TimeLimit << ',' << c.MemoryLimit << ',' << encoder(c.ResultDetail)
		     << ',' << c.CaseScore << "],";

	json << "[]]}";
}