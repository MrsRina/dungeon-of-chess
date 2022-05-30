#include "tessellator.h"
#include "util.h"

GLuint tessellator::draw_mode          = 0;
uint32_t tessellator::draw_count       = 0;
GLuint tessellator::buffer_vertex      = 0;
GLuint tessellator::buffer_material    = 0;
GLuint tessellator::attribute_vertex   = 0;
GLuint tessellator::attribute_material = 0;
GLuint tessellator::bind_texture_id    = 0;
shader tessellator::tessellator_shader = shader();

void tessellator::init() {
	if (!shader_manager::compile_shader(tessellator_shader, "data/fx/fx_tessellator.vsh", "data/fx/fx_tessellator.fsh")) {
		util::log("Failed to compile tessellator fx.");
	}

	glGenBuffers(1, &buffer_vertex);
	glGenBuffers(1, &buffer_material);

	tessellator_shader.use();
	attribute_vertex = glGetAttribLocation(tessellator_shader.program, "attribute_pos");
	attribute_material = glGetAttribLocation(tessellator_shader.program, "attribute_material");
	tessellator_shader.end();

	util::log("Tessellator initialized.");
}

void tessellator::context(float* proj_matrix) {
	tessellator_shader.use();
	glUniformMatrix4fv(glGetUniformLocation(tessellator_shader.program, "proj_matrix"), 1, GL_FALSE, proj_matrix);
	tessellator_shader.end();
}

void tessellator::start(GLint mode, uint32_t count) {
	draw_mode = mode;
	draw_count = count;
	bind_texture_id = 0;
}

void tessellator::draw(uint32_t vertex_count, uint32_t material_count, GLfloat* vertex_data, GLfloat* material_data) {
	tessellator_shader.use();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (bind_texture_id != 0) {

	}

	glEnableVertexAttribArray(attribute_vertex);
	glEnableVertexAttribArray(attribute_material);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
	glVertexAttribPointer(buffer_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_count, vertex_data, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_material);
	glVertexAttribPointer(attribute_material, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * material_count, material_data, GL_DYNAMIC_DRAW);

	glDrawArrays(draw_mode, 0, draw_count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (bind_texture_id != 0) {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	tessellator_shader.end();
}

void tessellator::texture(GLuint texture_id) {
	bind_texture_id = texture_id;
}