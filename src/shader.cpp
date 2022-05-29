#include "shader.h"
#include "util.h"

GLuint shader_manager::check_vertex_shader(const char* vertex_shader_path) {
	std::string data = util::file::read(vertex_shader_path);
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, data.c_str(), NULL);
	glCompileShader(vertex_shader);

	GLint shader_compile_status = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_compile_status);

	if (shader_compile_status != GL_TRUE) {
		char* log = new char[1024];
		glGetShadrInfoLog(vertex_shader, 1024, NULL, &log);

		util::log(std::to_string(log));
		delete[] log;

		return 0;
	}

	return vertex_shader;
}

GLuint shader_manager::check_fragment_shader(const char* fragment_shader_path) {
	std::string data = util::file::read(fragment_shader_path);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragment_shader, 1, data, NULL);
	glCompileShader(fragment_shader);

	GLint shader_compile_status = GL_FALSE;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_compile_status);

	if (shader_compile_status != GL_TRUE) {
		char* log = new char[1024];
		glGetShadrInfoLog(fragment_shader, 1024, NULL, &log);

		util::log(std::to_string(log));
		delete[] log;

		return 0;
	}

	return fragment_shader;
}

shader &shader_manager::compile_shader(const char* vertex_shader_path, const char* fragment_shader_path) {
	GLuint vertex_shader, fragment_shader;
	shader _shader;

	if (util::file::exists(vertex_shader_path)) {
		vertex_shader = this->check_vertex_shader(vertex_shader_path);
		_shader.vertex = vertex_shader;

		glAttachShader(this->program, vertex_shader);
	}

	if (util::file::exists(fragment_shader_path)) {
		fragment_shader = this->check_vertex_shader(vertex_shader_path);
		_shader.fragment = fragment_shader;

		glAttachShader(this->program, fragment_shader);
	}

	return _shader;
}