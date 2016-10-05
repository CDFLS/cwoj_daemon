#include <string>
#include <sstream>
#include "judge_daemon.h"

class encoder {
	char *buf;
public:
	encoder(std::string &in) {
		buf = (char *) malloc((in.size() << 1) + 3);
		if (!buf)
			throw "encoder: Can't allocate memory";
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

void json_builder(std::ostringstream &json, solution *target) {
	if (target->TimeStamp != 0)
		json << "{\"state\":\"finish\",\"detail\":[";
	else
		json << "{\"state\":\"wait\",\"detail\":[";
	/*if(target->error_code == RES_CE){
		json<<RES_CE<<",0,0,"<<encoder(target->last_state)<<"]}";
		return;
	}*/
	for (SingleTestCaseReport &c : target->TestCaseDetail)
		json << "[" << c.ErrorCode << ',' << c.TimeProfile << ',' << c.MemoryProfile << ',' << encoder(c.AdditionalInformation) << ',' << c.CaseScore
		     << "],";

	json << "[]]}";
}