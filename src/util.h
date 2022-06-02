#pragma once

#include "includes.h"

#ifndef UTIL_H
#define UTIL_H

static float MASK_SHAPE_VERTEX[18];
static float MASK_SHAPE_MATERIAL_COLOR[24];
static float MASK_SHAPE_MATERIAL[12];

struct util {
	static void log(const std::string &log_str);

	struct color {
		enum format {
			RGBA, ALPHA, EMPTY
		};

		format phase;
		uint8_t r, g, b, a;

		color() {
			this->phase = format::EMPTY;
		}

		color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) {
			this->r = red;
			this->g = green;
			this->b = blue;
			this->a = alpha;
			this->phase = format::RGBA;
		}

		color(uint8_t alpha) {
			this->a = alpha;
			this->phase = format::ALPHA;
		}

		void set(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
		void set(uint8_t alpha);

		float redf();
		float greenf();
		float bluef();
		float alphaf();
	};

	struct texture {
		bool loaded;
		GLuint id;

		uint32_t width;
		uint32_t height;
	};

	struct file {
		static bool exists(const char* path);
		static std::string read(const char* path);
		static bool read_texture(util::texture &texture, GLenum format, const char* path);
	};

	struct math {
		/**
		 * I took from wiki.
		 **/
		static void ortho2d(float* mat, float left, float right, float bottom, float top);
		static int32_t clampi(int32_t val, int32_t min, int32_t max);
		static float clampf(float val, float min, float max);
		static float lerpf(float a, float b, float t);
	};

	struct render {
		static void shape(float x, float y, float w, float h, util::color color);
		static void shape_outline(float x, float y, float w, float h, float line_thickness, util::color color);
		static void shape_texture(float x, float y, float w, float h, float tx, float ty, float tw, float th, util::texture &texture, util::color color = util::color());
	};
};

#endif