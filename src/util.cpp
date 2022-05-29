#include "util.h"

void util::log(const std::string &log_str) {
	std::cout << log_str.c_str() << "\n";
}

bool util::file::exists(const char* path) {
	FILE* f = fopen(path, "r");
	bool flag = f != NULL;

	if (flag) {
		fclose(f);
	}

	return flag;
}

std::string util::file::read(const char* path) {
	FILE* f = fopen(path, "r");

	size_t n = 0;
	int32_t c;

	if (f == NULL) {
		return "";
	}

	fseek(f, 0, SEEK_END);
	uint64_t f_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char code[f_size];

	while((c = fgetc(f)) != EOF) {
		code[n++] = (char) c;
	}

	fclose(f);
	
	std::string copy(code);
	return copy;
}