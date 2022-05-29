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
	std::ifstream ifs(path);
	return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}