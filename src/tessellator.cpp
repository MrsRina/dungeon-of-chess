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
bool tessellator::texture_color_filter    = false;
shader tessellator::tessellator_shader    = shader();
util::color tessellator::concurrent_color = util::color(0, 0, 0, 0);

std::vector<GLfloat> tessellator::concurrent_vertex_data = {};
std::vector<GLfloat> tessellator::concurrent_material_data = {};

void tessellator::init() {
	tessellator_shader = fx_manager::default_fx;

	glGenBuffers(1, &buffer_vertex);
	glGenBuffers(1, &buffer_material);

	util::log("Tessellator initialized.");
}

void tessellator::start(GLint mode, uint32_t count) {
	draw_mode            = mode;
	draw_count           = count;
	bind_texture_id      = 0;
	texture_color_filter = false;

	concurrent_vertex_data.clear();
	concurrent_material_data.clear();
}

void tessellator::vertex(float x, float y, float z) {
	concurrent_vertex_data.push_back(x);
	concurrent_vertex_data.push_back(y);
	concurrent_vertex_data.push_back(z);
}

void tessellator::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if (bind_texture_id == 0) {
		concurrent_material_data.push_back((float) r / 255.0f);
		concurrent_material_data.push_back((float) g / 255.0f);
		concurrent_material_data.push_back((float) b / 255.0f);
		concurrent_material_data.push_back((float) a / 255.0f);
	} else {
		concurrent_color.set(r, g, b, a);
		texture_color_filter = true;
	}
}

void tessellator::color(util::color color) {
	if (bind_texture_id != 0) {
		concurrent_color = color;
		texture_color_filter = true;
	} else {
		concurrent_material_data.push_back(color.redf());
		concurrent_material_data.push_back(color.greenf());
		concurrent_material_data.push_back(color.bluef());
		concurrent_material_data.push_back(color.alphaf());
	}
}

void tessellator::color(uint8_t alpha) {
	if (bind_texture_id != 0) {
		concurrent_color.a = alpha;
	}
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

	tessellator_shader.set_bool("texture_status", false);
	tessellator_shader.set_bool("texture_color_filter", texture_color_filter);

	if (bind_texture_id != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bind_texture_id);

		tessellator_shader.set_bool("texture_status", true);
		tessellator_shader.set_int("texture_index", 0);
		
		tessellator_shader.set_float("texture_color_r", concurrent_color.redf());
		tessellator_shader.set_float("texture_color_g", concurrent_color.greenf());
		tessellator_shader.set_float("texture_color_b", concurrent_color.bluef());
		tessellator_shader.set_float("texture_color_a", concurrent_color.alphaf());
	}

	glEnableVertexAttribArray(attribute_vertex);
	glEnableVertexAttribArray(attribute_material);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * concurrent_vertex_data.size(), &concurrent_vertex_data[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_material);
	glVertexAttribPointer(attribute_material, bind_texture_id != 0 ? 2 : 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
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
	draw_mode            = mode;
	draw_count           = count;
	draw_vertex_count    = vertex_count;
	draw_material_count  = material_count;
	bind_texture_id      = 0;
	texture_color_filter = false;
}

void tessellator::draw(GLfloat* vertex_data, GLfloat* material_data) {
	tessellator_shader.use();

	attribute_vertex = glGetAttribLocation(tessellator_shader.program, "attribute_pos");
	attribute_material = glGetAttribLocation(tessellator_shader.program, "attribute_material");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tessellator_shader.set_bool("texture_status", false);
	tessellator_shader.set_bool("texture_color_filter", texture_color_filter);
	
	if (bind_texture_id != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bind_texture_id);

		tessellator_shader.set_bool("texture_status", true);
		tessellator_shader.set_int("texture_index", 0);

		tessellator_shader.set_float("texture_color_r", concurrent_color.redf());
		tessellator_shader.set_float("texture_color_g", concurrent_color.greenf());
		tessellator_shader.set_float("texture_color_b", concurrent_color.bluef());
		tessellator_shader.set_float("texture_color_a", concurrent_color.alphaf());
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
	color(255);
}

void tessellator::linethickness(float thickness) {
	glLineWidth(thickness);
}

void tessellator::fx(shader _shader) {
	tessellator_shader = _shader;
}