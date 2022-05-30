#pragma once

#include "includes.h"
#include "shader.h"

#ifndef TESSELLATOR_H
#define TESSELLATOR_H

class tessellator {
protected:
	static shader tessellator_shader;
	static GLuint draw_mode, bind_texture_id;
	static uint32_t draw_count;

	/* The attributes used in shader stuff. */
	static GLuint attribute_vertex;
	static GLuint attribute_material;

	/* The buffers. */
	static GLuint buffer_vertex;
	static GLuint buffer_material;
public:
	static void init();
	static void context(float* proj_matrix);
	static void texture(GLuint texture_id);
	static void start(GLint draw_mode, uint32_t draw_count);
	static void draw(uint32_t vertex_count, uint32_t material_count, GLfloat* vertex_data, GLfloat* material_data);
};

#endif