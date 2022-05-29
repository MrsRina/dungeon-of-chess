#pragma once

#include "includes.h"
#include "shader.h"

class tessellator {
protected:
	static shader tessellator_shader;
	static GLuint draw_mode;
	static uint32_t draw_count;
public:
	static void start(GLint draw_mode, uint32_t draw_count);
	static void draw();
};