#include "util.h"

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