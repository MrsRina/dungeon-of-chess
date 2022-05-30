#include "shader.h"
#include "util.h"

void shader::use() {
	glUseProgram(this->program);
}

void shader::end() {
	glUseProgram(0);
}

GLuint shader_manager::check_compile(GLint mode, const char* shader_path) {
	std::string data_str = util::file::read(shader_path);
	const char* data = data_str.c_str();

	GLuint shader = glCreateShader(mode);

	glShaderSource(shader, 1, &data, NULL);
	glCompileShader(shader);

	GLint shader_compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);

	if (shader_compile_status != GL_TRUE) {
		char log[256];
		glGetShaderInfoLog(shader, 256, NULL, log);

		util::log(shader_path);
		util::log(log);
		return 0;
	}

	return shader;
}

bool shader_manager::compile_shader(shader &_shader, const char* vertex_shader_path, const char* fragment_shader_path) {
	GLuint vertex_shader, fragment_shader;

	_shader.compiled = true;

	if (util::file::exists(vertex_shader_path)) {
		vertex_shader = check_compile(GL_VERTEX_SHADER, vertex_shader_path);
		_shader.vertex = vertex_shader;

		if (vertex_shader != 0) {
			util::log(std::string(vertex_shader_path) + " compiled.");
		} else {
			_shader.compiled = false;
		}
	} else {
		_shader.compiled = false;
	}

	if (util::file::exists(fragment_shader_path)) {
		fragment_shader = check_compile(GL_FRAGMENT_SHADER, fragment_shader_path);
		_shader.fragment = fragment_shader;

		if (fragment_shader != 0) {
			util::log(std::string(fragment_shader_path) + " compiled.");
		} else {
			_shader.compiled = false;
		}
	} else {
		_shader.compiled = false;
	}

	if (_shader.compiled) {
		_shader.program = glCreateProgram();

		glAttachShader(_shader.program, vertex_shader);
		glAttachShader(_shader.program, fragment_shader);
		glLinkProgram(_shader.program);

		GLint compiled = GL_FALSE;
		glGetProgramiv(_shader.program, GL_LINK_STATUS, &compiled);

		if (compiled == 0) {
			char log[256];
			glGetProgramInfoLog(_shader.program, 256, NULL, log);

			util::log("Failed to link program.");
			util::log(log);
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	return _shader.compiled;
}