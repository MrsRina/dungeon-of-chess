#version 330 core

in vec4 varying_material;
in vec4 varying_pos;

uniform float texture_color_r, texture_color_g, texture_color_b, texture_color_a;
uniform bool texture_status, texture_color_filter;
uniform sampler2D texture_index;

uniform float x, y, viewport_h, scale = 1.0f;

void main() {
	vec4 fragment;

	if (texture_status) {
		fragment = texture2D(texture_index, vec2(varying_material.x, varying_material.y));

		// Set to negative color.
		float r = 1.0f - texture_color_r;
		float g = 1.0f - texture_color_g;
		float b = 1.0f - texture_color_b;

		// Force one color or set everything to the texture color.
		if (texture_color_filter) {
			fragment.r = (fragment.r - (r - 1.0f));
			fragment.g = (fragment.g - (g - 1.0f));
			fragment.b = (fragment.b - (b - 1.0f));
		}

		// The alpha channel.
		fragment.a = (fragment.a - (1.0f - texture_color_a));
	} else {
		fragment = varying_material;
	}

	// Get the fragment position.
	vec2 pos = vec2(gl_FragCoord.x, viewport_h - gl_FragCoord.y);

	// Subtract with mouse position.
	pos.x = pos.x - x;
	pos.y = pos.y - y;

	// Get the alpha difference by distance.
	float alpha_diff = 255 - clamp(sqrt(pos.x * pos.x + pos.y * pos.y) / scale, 0, 255);

	// Set new fragment alpha.
	fragment.a = (fragment.a - (1.0f - (alpha_diff / 255.0f)));

	// Set fragment color.
	gl_FragColor = fragment;
}