#pragma once

#include "includes.h"

struct util {
	static void log(const std::string &log_str);

	struct file {
		static bool exits(const char* path);
		static std::string &read(const char* path);
	}
};

class tessellator {
public:
	static void start(GLint draw_mode, uint32_t draw_count);
	static void draw();
};