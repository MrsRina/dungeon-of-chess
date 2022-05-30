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

	struct math {
		/**
		 * I took from wiki.
		 **/
		static void ortho2d(float* mat, float left, float right, float bottom, float top);
	};
};

#endif