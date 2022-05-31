#include "shader.h"
#include "util.h"

float shader_manager::matrix_proj_ortho[16];
float shader_manager::viewport[4];

void shader::use() {
	glUseProgram(this->program);
}

void shader::end() {
	glUseProgram(0);
}

void shader::set_matrix(float* matrix) {
	glUniformMatrix4fv(glGetUniformLocation(this->program, "matrix"), 1, GL_FALSE, matrix);
}

void shader::set_bool(const std::string &name, bool value) {
	glUniform1i(glGetUniformLocation(this->program, name.c_str()), (int) value);
};

void shader::set_int(const std::string &name, int value) {
	glUniform1i(glGetUniformLocation(this->program, name.c_str()), value);
};

void shader::set_float(const std::string &name, float value) {
	glUniform1f(glGetUniformLocation(this->program, name.c_str()), value);
};

void shader_manager::context() {
	glGetFloatv(GL_VIEWPORT, viewport);
	util::math::ortho2d(shader_manager::matrix_proj_ortho, 0.0f, viewport[2], viewport[3], 0.0f);
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

shader fx_manager::mouse_outline_fx  = shader();
shader fx_manager::light_specular_fx = shader();
shader fx_manager::default_fx        = shader();

void fx_manager::init() {
	if (!shader_manager::compile_shader(mouse_outline_fx, "data/fx/fx_mouse_outline.vsh", "data/fx/fx_mouse_outline.fsh")) {
		util::log("Could not load 'mouse outline' fx.");
	}

	if (!shader_manager::compile_shader(light_specular_fx, "data/fx/fx_light_specular.vsh", "data/fx/fx_light_specular.fsh")) {
		util::log("Could not load 'light specular' fx.");
	}

	if (!shader_manager::compile_shader(default_fx, "data/fx/fx_default.vsh", "data/fx/fx_default.fsh")) {
		util::log("Could not load 'default' fx.");
	}

	util::log("All fx effects initialized.");
}

void fx_manager::context() {
	mouse_outline_fx.use();
	mouse_outline_fx.set_matrix(shader_manager::matrix_proj_ortho);
	mouse_outline_fx.set_float("viewport_h", shader_manager::viewport[3]);

	default_fx.use();
	default_fx.set_matrix(shader_manager::matrix_proj_ortho);

	light_specular_fx.use();
	light_specular_fx.set_matrix(shader_manager::matrix_proj_ortho);
	light_specular_fx.set_float("viewport_h", shader_manager::viewport[3]);

	// or shade::end();
	glUseProgram(0);
}