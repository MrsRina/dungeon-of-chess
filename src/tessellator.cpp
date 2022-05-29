#include "tessellator.h"

GLuint tessellator::draw_mode = 0;
uint32_T tessellator::draw_count = 0;
shader tessellator::tessellator_shader = 0;

void tessellator::init() {
	tessellator_shader = shader_manager::compile_shader("data/fx/fx_tessellator.vsh", "data/fx/fx_tessellator.fsh");
}