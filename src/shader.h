#pragma once

#include "includes.h"

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

struct shader {
	GLuint program, vertex, fragment;
	bool compiled;

	void use();
	void end();
};

class shader_manager {
public:
	static void init();

	static GLuint check_compile(GLint mode, const char* shader_path);
	static bool compile_shader(shader &_shader, const char* vertex_shader_path, const char* fragment_shader_path);
};

#endif