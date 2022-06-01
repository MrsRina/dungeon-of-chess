#version 330 core

in vec4 varying_material;
in vec4 varying_pos;

uniform float texture_color_r, texture_color_g, texture_color_b, texture_color_a;
uniform bool texture_status, texture_color_filter;
uniform sampler2D texture_index;

uniform float mx, my;
uniform float viewport_h;

void main() {
	if (texture_status) {
		gl_FragColor = texture2D(texture_index, vec2(varying_material.x, varying_material.y));

		// Set to negative color.
		float r = 1.0f - texture_color_r;
		float g = 1.0f - texture_color_g;
		float b = 1.0f - texture_color_b;

		// Force one color or set everything to the texture color.
		if (texture_color_filter) {
			gl_FragColor.r = (gl_FragColor.r - (r - 1.0f));
			gl_FragColor.g = (gl_FragColor.g - (g - 1.0f));
			gl_FragColor.b = (gl_FragColor.b - (b - 1.0f));
		}

		// The alpha channel.
		gl_FragColor.a = (gl_FragColor.a - (1.0f - texture_color_a));
	} else {
		gl_FragColor = varying_material;
	}

	// Get the fragment position.
	vec2 pos = vec2(gl_FragCoord.x, viewport_h - gl_FragCoord.y);

	// Subtract with mouse position.
	pos.x = pos.x - mx;
	pos.y = pos.y - my;

	// Get the alpha difference by distance.
	float alpha_diff = 255 - clamp(sqrt(pos.x * pos.y + pos.y * pos.y), 0, 255);

	// Set new fragment alpha.
	gl_FragColor.a = alpha_diff / 255.0f;
}