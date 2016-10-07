/*
struct validator_info validator(FILE* fstd, FILE* fuser);

struct validator_info{
	int ret;
	char* user_mismatch;
	char* std_mismatch
};
 
	!!!!!!!!!!!!!!!!!!!!!
	REMEMBER TO FREE THESE TWO BUFFERS IN VAL_T!!!
	!!!!!!!!!!!!!!!!!!!!!

*/
#include "judge_daemon.h"

#include <math.h>


//#define VAL_FUCKED (-1)
//#define VAL_IDENTICAL 0
//#define VAL_MISMATCH 1
//#define VAL_LONGER 2
//#define VAL_SHORTER 3

#define BEGIN_OF_FILE -2

#define LENGTH_OF_MISMATCH_INFO 16

int rahcteg(FILE *stream);

int BOF(FILE *stream) {
	return ftell(stream) == 0;
}

int isending(int c) {
	return c == EOF || c == '\n';
}

void read_around(FILE *stream, char *buf) {
	int len;

	for (len = LENGTH_OF_MISMATCH_INFO / 2; !BOF(stream) && len; len--)
		rahcteg(stream);

	for (len = 0; len < LENGTH_OF_MISMATCH_INFO - 1; len++) {
		*buf = getc(stream);
		if (*buf == EOF)
			break;
		buf++;
	}
	*buf = 0;
}

struct ValidatorInfo mismatch(FILE *fstd, FILE *fuser) {
	struct ValidatorInfo ret;
	ret.Result = VALIDATOR_MISMATCH;
	ret.UserMismatch = (char *) malloc(LENGTH_OF_MISMATCH_INFO);
	ret.StandardMismatch = (char *) malloc(LENGTH_OF_MISMATCH_INFO);

	if (!ret.UserMismatch || !ret.StandardMismatch) {
		ret.Result = VALIDATOR_FUCKED;
		return ret;
	}

	read_around(fstd, ret.StandardMismatch);
	read_around(fuser, ret.UserMismatch);

	return ret;

}

struct field {
	FILE *fp;
	long st, ed;
	long length;
};

void find_end(struct field *f) {
	fseek(f->fp, f->st, SEEK_SET);
	int c;
	while (!isending(c = getc(f->fp)));
	f->ed = ftell(f->fp) - 1;
}

long filelen(FILE *stream) {
	fseek(stream, 0, SEEK_END);
	return ftell(stream);
}


long filelines(FILE *stream) {
	fseek(stream, 0, SEEK_SET);
	int c, tmp = 0, sum = 0, non_empty = 0;
	while (c = getc(stream)) {
		if (c == EOF)
			break;
		if (c == '\n') {
			tmp++;
		} else {
			sum += tmp;
			tmp = 0;
			non_empty = 1;
		}
	}
	return sum + non_empty;
}

struct field init_field(FILE *fp) {
	struct field ret;
	ret.fp = fp;
	ret.st = 0;
	find_end(&ret);
	ret.length = filelen(fp);
	return ret;
}

int eof(struct field *p) {
	return p->st >= p->length;
}

int rahcteg(FILE *stream) {
	int ret = getc(stream);
	fseek(stream, -2, SEEK_CUR);
	return ret;
}

long trip(struct field *fa) {
	long end_a;

	fseek(fa->fp, fa->ed, SEEK_SET);
	while (ftell(fa->fp) > fa->st
	       && isspace(rahcteg(fa->fp)));
	if (ftell(fa->fp) == fa->st) {
		if (isspace(getc(fa->fp)))
			end_a = fa->st - 1;
		else
			end_a = fa->st;
	} else {
		getc(fa->fp);
		end_a = ftell(fa->fp);
	}
	return end_a;
}

int compare(struct field *fa, struct field *fb) {
	/*find the available area*/
	long end_a = trip(fa);
	long end_b = trip(fb);

	/*compare the length of available area*/
	if ((end_a - fa->st) != (end_b - fb->st))
		return 1;

	fseek(fa->fp, fa->st, SEEK_SET);
	fseek(fb->fp, fb->st, SEEK_SET);

	int c = end_a - fa->st + 1;
	while (c--)
		if (getc(fa->fp) != getc(fb->fp))
			return 1;

	return 0;
}

ValidatorInfo validator(FILE *fstd, FILE *fuser) {
	struct ValidatorInfo ret;
	ret.Result = 0;
	ret.UserMismatch = ret.StandardMismatch = NULL;

	if (!fstd || !fuser) {
		ret.Result = VALIDATOR_FUCKED;
		return ret;
	}

	//if the numbers of lines don't match, return directly.
	int line_u = filelines(fuser);
	int line_s = filelines(fstd);
	if (line_u < line_s)
		ret.Result = VALIDATOR_SHORTER;
	else if (line_u > line_s)
		ret.Result = VALIDATOR_LONGER;
	if (line_u != line_s)
		return ret;

	struct field fs = init_field(fstd);
	struct field fu = init_field(fuser);

	int x;
	for (x = 0; x < line_u; x++) {
		if (compare(&fs, &fu))
			return mismatch(fs.fp, fu.fp);
		fs.st = fs.ed + 1;
		find_end(&fs);

		fu.st = fu.ed + 1;
		find_end(&fu);
	}
	ret.Result = VALIDATOR_IDENTICAL;
	return ret;
}

struct ValidatorInfo validator_int(FILE *fstd, FILE *fuser) {
	struct ValidatorInfo info;

	if (!fstd || !fuser) {
		info.Result = VALIDATOR_FUCKED;
		return info;
	}

#ifdef __MINGW32__
#define LL_FMT "%I64d"
#else
#define LL_FMT "%lld"
#endif

	for (;;) {
		int nstd, nusr;
		long long vstd, vusr;

		nstd = fscanf(fstd, LL_FMT, &vstd);
		nusr = fscanf(fuser, LL_FMT, &vusr);

		if (nstd == 1 && nusr == 1) {
			if (vstd != vusr) {
				info.Result = VALIDATOR_MISMATCH;
				info.UserMismatch = (char *) malloc(25);
				info.StandardMismatch = (char *) malloc(25);

				sprintf(info.UserMismatch, LL_FMT, vusr);
				sprintf(info.StandardMismatch, LL_FMT, vstd);

				break;
			}
		} else if (nstd == 1) {
			info.Result = VALIDATOR_SHORTER;
			break;
		} else if (nusr == 1) {
			info.Result = VALIDATOR_LONGER;
			break;
		} else {
			info.Result = VALIDATOR_IDENTICAL;
			break;
		}
	}

	return info;
}

const double error_tab[10] =
		{1, 0.1, 0.01, 0.001, 0.0001, 0.00001,
		 0.000001, 0.0000001, 0.00000001, 0.000000001};

struct ValidatorInfo validator_float(FILE *fstd, FILE *fuser, int prec) {
	struct ValidatorInfo info;
	double error;

	if (!fstd || !fuser || prec < 0 || prec > 9) {
		info.Result = VALIDATOR_FUCKED;
		return info;
	}
	error = error_tab[prec];
	for (;;) {
		int nstd, nusr;
		double vstd, vusr;

		nstd = fscanf(fstd, "%lf", &vstd);
		nusr = fscanf(fuser, "%lf", &vusr);

		if (nstd == 1 && nusr == 1) {
			if (fabs(vstd - vusr) >= error) {
				info.Result = VALIDATOR_MISMATCH;
				info.UserMismatch = (char *) malloc(64);
				info.StandardMismatch = (char *) malloc(64);

				sprintf(info.UserMismatch, "%.10lf", vusr);
				sprintf(info.StandardMismatch, "%.10lf", vstd);

				break;
			}
		} else if (nstd == 1) {
			info.Result = VALIDATOR_SHORTER;
			break;
		} else if (nusr == 1) {
			info.Result = VALIDATOR_LONGER;
			break;
		} else {
			info.Result = VALIDATOR_IDENTICAL;
			break;
		}
	}
	return info;
}
/*
int main()
{
	FILE* f1 = fopen("A.txt", "r");
	FILE* f2 = fopen("B.txt", "r");
	struct validator_info ret = validator(f1, f2);

	if(ret.ret == VAL_FUCKED)
		printf("Error\n");
	else if(ret.ret == VAL_IDENTICAL)
		printf("Identical\n");
	else if(ret.ret == VAL_MISMATCH){
		printf("Mismatch\n");
		printf("user output: %s\n", ret.user_mismatch);
		printf("std output: %s\n", ret.std_mismatch);
	}else if(ret.ret == VAL_SHORTER)
		printf("Shorter\n");
	else
		printf("Longer\n");
	return 0;
}
*/
