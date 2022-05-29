#pragma once

#include "includes.h"

struct util {
	static void log(const std::string &log_str);

	struct file {
		static bool exists(const char* path);
		static std::string read(const char* path);
	};
};