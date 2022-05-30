#include "util.h"
#include "tessellator.h"

float util::color::redf() {
	return (float) this->r / 255.0f;
}

float util::color::greenf() {
	return (float) this->g / 255.0f;
}

float util::color::bluef() {
	return (float) this->b / 255.0f;
}

float util::color::alphaf() {
	return (float) this->a / 255.0f;
}

void util::log(const std::string &log_str) {
	std::string formated = "[MAIN] " + log_str;
	std::cout << formated.c_str() << "\n";
}

bool util::file::exists(const char* path) {
	FILE* f = fopen(path, "r");
	bool flag = f != NULL;

	if (flag) {
		fclose(f);
	}

	return flag;
}

std::string util::file::read(const char* path) {
	std::ifstream ifs(path);
	std::string string_builder;

	if (ifs.is_open()) {
		std::string string_buffer;

		while (getline(ifs, string_buffer)) {
			string_builder += "\n" + string_buffer;
		}

		ifs.close();
	}

	return string_builder;
}

bool util::file::read_texture(util::texture &texture, GLenum format, const char* path) {
	SDL_Surface* surface = IMG_Load(path);

	if (surface == NULL) {
		util::log(std::string(path) + " could not load image.");
		return false;
	}

	texture.width = surface->w;
	texture.height = surface->h;

	// Create texture id and bind it.
	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);

	GLint internal_format = format;

	// dispatch texture into "memory".
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free surface after use.
	SDL_FreeSurface(surface);

	return true;
}

void util::math::ortho2d(float* mat, float left, float right, float bottom, float top) {
    const float zNear = -1.0f;
    const float zFar = 1.0f;
    const float inv_z = 1.0f / (zFar - zNear);
    const float inv_y = 1.0f / (top - bottom);
    const float inv_x = 1.0f / (right - left);

    // first column
    *mat++ = (2.0f * inv_x);
    *mat++ = (0.0f);
    *mat++ = (0.0f);
    *mat++ = (0.0f);

    // second
    *mat++ = (0.0f);
    *mat++ = (2.0 * inv_y);
    *mat++ = (0.0f);
    *mat++ = (0.0f);

    // third
    *mat++ = (0.0f);
    *mat++ = (0.0f);
    *mat++ = (-2.0f * inv_z);
    *mat++ = (0.0f);

    // fourth
    *mat++ = (-(right + left) * inv_x);
    *mat++ = (-(top + bottom) * inv_y);
    *mat++ = (-(zFar + zNear) * inv_z);
    *mat++ = (1.0f);
}

void util::render::shape(float x, float y, float w, float h, util::color color) {
	uint8_t i = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	float r = color.redf(), g = color.greenf(), b = color.bluef(), a = color.alphaf();
	i = 0;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	tessellator::start(GL_TRIANGLE_STRIP, 6, 18, 24);
	tessellator::draw(mask_shape_vertex, mask_shape_material_color);
}

void util::render::shape_outline(float x, float y, float w, float h, float line_thickness, util::color color) {
	uint8_t i = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y + h;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x + w;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	mask_shape_vertex[i++] = x;
	mask_shape_vertex[i++] = y;
	mask_shape_vertex[i++] = 0;

	float r = color.redf(), g = color.greenf(), b = color.bluef(), a = color.alphaf();
	i = 0;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	mask_shape_material_color[i++] = r;
	mask_shape_material_color[i++] = g;
	mask_shape_material_color[i++] = b;
	mask_shape_material_color[i++] = a;

	tessellator::start(GL_LINE_STRIP, 6, 18, 24);
	tessellator::linethickness(line_thickness);
	tessellator::draw(mask_shape_vertex, mask_shape_material_color);
}

void util