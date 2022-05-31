#pragma once

#include "includes.h"

#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

struct shader {
	GLuint program, vertex, fragment;
	bool compiled;

	void use();
	void end();

	void set_matrix(float* matrix);
	void set_bool(const std::string &name, bool value);
	void set_int(const std::string &name, int value);
	void set_float(const std::string &name, float value);
};

class shader_manager {
public:
	static float matrix_proj_ortho[16];
	static float viewport[4];

	static void init();
	static void context();

	static GLuint check_compile(GLint mode, const char* shader_path);
	static bool compile_shader(shader &_shader, const char* vertex_shader_path, const char* fragment_shader_path);
};

class fx_manager {
public:
	static shader mouse_outline_fx;
	static shader light_specular_fx;
	static shader default_fx;

	static void init();
	static void context();
};

#endif