#pragma once

#include "includes.h"

#ifndef UTIL_H
#define UTIL_H

struct util {
	static void log(const std::string &log_str);

	struct file {
		static bool exists(const char* path);
		static std::string read(const char* path);
	};
};

#endif