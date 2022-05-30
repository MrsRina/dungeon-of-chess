#pragma once

#include "includes.h"
#include "shader.h"

#ifndef TESSELLATOR_H
#define TESSELLATOR_H

class tessellator {
protected:
	static shader tessellator_shader;
	static GLuint draw_mode, bind_texture_id;
	static uint32_t draw_vertex_count, draw_material_count, draw_count;

	/* The attributes used in shader stuff. */
	static GLuint attribute_vertex;
	static GLuint attribute_material;

	/* The buffers. */
	static GLuint buffer_vertex;
	static GLuint buffer_material;

	/* Instead build with a pre defined array, we create a mesh. */
	static std::vector<GLfloat> concurrent_vertex_data;
	static std::vector<GLfloat> concurrent_material_data;
public:
	static void init();
	static void texture(GLuint texture_id);
	static void start(GLint draw_mode, uint32_t draw_count);
	static void start(GLint draw_mode, uint32_t draw_count, uint32_t draw_vertex_count, uint32_t draw_material_count);
	static void vertex(float x, float y, float z);
	static void color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void draw(GLfloat* vertex_data, GLfloat* material_data);
	static void draw();
	static void fx(shader _shader = fx_manager::default_fx);
	static void linethickness(float thickness);
};

#endif