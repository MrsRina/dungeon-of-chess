#include "tessellator.h"
#include "util.h"

GLuint tessellator::draw_mode = 0;
uint32_t tessellator::draw_count = 0;
shader tessellator::tessellator_shader = shader();

void tessellator::init() {
	if (!shader_manager::compile_shader(tessellator_shader, "data/fx/fx_tessellator.vsh", "data/fx/fx_tessellator.fsh")) {
		util::log("Failed to compile tessellator fx.");
	}

	util::log("Tessellator initialized.");
}