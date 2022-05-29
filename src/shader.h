#pragma once

#include "includes.h"

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

struct shader {
	GLuint program, vertex, fragment;
	bool compiled;
};

class shader_manager {
public:
	static void init();

	static GLuint check_vertex_shader(const char* vertex_shader_path);
	static GLuint check_fragment_shader(const char* fragment_shader_path);

	static bool compile_shader(shader &_shader, const char* vertex_shader_path, const char* fragment_shader_path);
};

#endif