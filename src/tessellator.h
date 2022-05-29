#pragma once

#include "includes.h"
#include "shader.h"

#ifndef TESSELLATOR_H
#define TESSELLATOR_H

class tessellator {
protected:
	static shader tessellator_shader;
	static GLuint draw_mode;
	static uint32_t draw_count;
public:
	static void init();
	static void start(GLint draw_mode, uint32_t draw_count);
	static void draw();
};

#endif