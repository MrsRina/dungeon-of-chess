#pragma once

#include "includes.h"

#ifndef UTIL_H
#define UTIL_H

static float mask_shape_vertex[18];
static float mask_shape_material_color[24];
static float mask_shape_material[12];

struct util {
	static void log(const std::string &log_str);

	struct color {
		uint8_t r, g, b, a;

		color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) {
			this->r = red;
			this->g = green;
			this->b = blue;
			this->a = alpha;
		}

		float redf();
		float greenf();
		float bluef();
		float alphaf();
	};

	struct file {
		static bool exists(const char* path);
		static std::string read(const char* path);
	};

	struct math {
		/**
		 * I took from wiki.
		 **/
		static void ortho2d(float* mat, float left, float right, float bottom, float top);
	};

	struct render {
		static void shape(float x, float y, float w, float h, util::color color);
		static void shape_outline(float x, float y, float w, float h, float line_thickness, util::color color);
	};
};

#endif