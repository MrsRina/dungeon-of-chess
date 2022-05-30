#include "tessellator.h"
#include "util.h"

GLuint tessellator::draw_mode             = 0;
uint32_t tessellator::draw_count          = 0;
uint32_t tessellator::draw_vertex_count   = 0;
uint32_t tessellator::draw_material_count = 0;
GLuint tessellator::buffer_vertex         = 0;
GLuint tessellator::buffer_material       = 0;
GLuint tessellator::attribute_vertex      = 0;
GLuint tessellator::attribute_material    = 0;
GLuint tessellator::bind_texture_id       = 0;
shader tessellator::tessellator_shader    = shader();

std::vector<GLfloat> tessellator::concurrent_vertex_data = {};
std::vector<GLfloat> tessellator::concurrent_material_data = {};

void tessellator::init() {
	tessellator_shader = fx_manager::default_fx;

	glGenBuffers(1, &buffer_vertex);
	glGenBuffers(1, &buffer_material);

	util::log("Tessellator initialized.");
}

void tessellator::start(GLint mode, uint32_t count) {
	draw_mode = mode;
	draw_count = count;
	bind_texture_id = 0;

	concurrent_vertex_data.clear();
	concurrent_material_data.clear();
}

void tessellator::vertex(float x, float y, float z) {
	concurrent_vertex_data.push_back(x);
	concurrent_vertex_data.push_back(y);
	concurrent_vertex_data.push_back(z);
}

void tessellator::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	concurrent_material_data.push_back((float) r / 255.0f);
	concurrent_material_data.push_back((float) g / 255.0f);
	concurrent_material_data.push_back((float) b / 255.0f);
}

void tessellator::uv(float u, float v) {
	concurrent_material_data.push_back(u);
	concurrent_material_data.push_back(v);
}

void tessellator::draw() {
	tessellator_shader.use();

	attribute_vertex = glGetAttribLocation(tessellator_shader.program, "attribute_pos");
	attribute_material = glGetAttribLocation(tessellator_shader.program, "attribute_material");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tessellator_shader.set_bool("contains_texture", false);
	
	if (bind_texture_id != 0) {
		tessellator_shader.set_bool("contains_texture", true);
		tessellator_shader.set_int("texture_sampler", bind_texture_id);
	}

	glEnableVertexAttribArray(attribute_vertex);
	glEnableVertexAttribArray(attribute_material);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * concurrent_vertex_data.size(), &concurrent_vertex_data[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_material);
	glVertexAttribPointer(attribute_material, bind_texture != 0 ? 2 : 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * concurrent_material_data.size(), &concurrent_material_data[0], GL_DYNAMIC_DRAW);

	glDrawArrays(draw_mode, 0, draw_count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (bind_texture_id != 0) {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	linethickness(1.0f);
	tessellator_shader.end();
}

void tessellator::start(GLint mode, uint32_t count, uint32_t vertex_count, uint32_t material_count) {
	draw_mode = mode;
	draw_count = count;
	draw_vertex_count = vertex_count;
	draw_material_count = material_count;
	bind_texture_id = 0;
}

void tessellator::draw(GLfloat* vertex_data, GLfloat* material_data) {
	tessellator_shader.use();

	attribute_vertex = glGetAttribLocation(tessellator_shader.program, "attribute_pos");
	attribute_material = glGetAttribLocation(tessellator_shader.program, "attribute_material");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tessellator_shader.set_bool("contains_texture", false);
	
	if (bind_texture_id != 0) {
		tessellator_shader.set_bool("contains_texture", true);
		tessellator_shader.set_int("texture_sampler", bind_texture_id);
	}

	glEnableVertexAttribArray(attribute_vertex);
	glEnableVertexAttribArray(attribute_material);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * draw_vertex_count, vertex_data, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_material);
	glVertexAttribPointer(attribute_material, bind_texture_id != 0 ? 2 : 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * draw_material_count, material_data, GL_DYNAMIC_DRAW);

	glDrawArrays(draw_mode, 0, draw_count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (bind_texture_id != 0) {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	linethickness(1.0f);
	tessellator_shader.end();
}

void tessellator::texture(GLuint texture_id) {
	bind_texture_id = texture_id;
}

void tessellator::linethickness(float thickness) {
	glLineWidth(thickness);
}

void tessellator::fx(shader _shader) {
	tessellator_shader = _shader;
}