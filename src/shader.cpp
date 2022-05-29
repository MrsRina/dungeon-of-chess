#include "shader.h"
#include "util.h"

GLuint shader_manager::check_vertex_shader(const char* vertex_shader_path) {
	std::string data_str = util::file::read(vertex_shader_path);
	const char* data = data_str.c_str();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, &data, NULL);
	glCompileShader(vertex_shader);

	GLint shader_compile_status = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_compile_status);

	if (shader_compile_status != GL_TRUE) {
		char log[1024];
		glGetShaderInfoLog(vertex_shader, 1024, NULL, log);

		util::log(vertex_shader_path);
		util::log(log);
		return 0;
	}

	return vertex_shader;
}

GLuint shader_manager::check_fragment_shader(const char* fragment_shader_path) {
	std::string data_str = util::file::read(fragment_shader_path);
	const char* data = data_str.c_str();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment_shader, 1, &data, NULL);
	glCompileShader(fragment_shader);

	GLint shader_compile_status = GL_FALSE;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_compile_status);

	if (shader_compile_status != GL_TRUE) {
		char log[1024];
		glGetShaderInfoLog(fragment_shader, 1024, NULL, log);

		util::log(fragment_shader_path);
		util::log(log);
		return 0;
	}

	return fragment_shader;
}

bool shader_manager::compile_shader(shader &_shader, const char* vertex_shader_path, const char* fragment_shader_path) {
	GLuint vertex_shader, fragment_shader;

	_shader.program = glCreateProgram();
	_shader.compiled = true;

	if (util::file::exists(vertex_shader_path)) {
		vertex_shader = check_vertex_shader(vertex_shader_path);
		_shader.vertex = vertex_shader;

		if (vertex_shader != 0) {
			glAttachShader(_shader.program, vertex_shader);
		} else {
			_shader.compiled = false;
		}
	} else {
		_shader.compiled = false;
	}

	if (util::file::exists(fragment_shader_path)) {
		fragment_shader = check_fragment_shader(fragment_shader_path);
		_shader.fragment = fragment_shader;

		if (fragment_shader != 0) {
			glAttachShader(_shader.program, fragment_shader);
		} else {
			_shader.compiled = false;
		}
	} else {
		_shader.compiled = false;
	}

	if (_shader.compiled) {
		glLinkProgram(_shader.program);

		GLint compiled = GL_FALSE;
		glGetProgramiv(_shader.program, GL_LINK_STATUS, &compiled);

		if (compiled != GL_TRUE) {
			util::log("Failed to link program.");
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	return _shader.compiled;
}