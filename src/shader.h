#pragma 

#include "includes.h"

struct shader {
	GLuint vertex, fragment;
	bool compiled;
};

class shader_manager {
protected:
	GLuint program;
public:
	void init();

	GLuint check_vertex_shader(const char* vertex_shader_path);
	GLuint check_fragment_shader(const char& fragment_shader_path);

	shader &compile_shader(const char* vertex_shader_path, const char* fragment_shader_path);
};