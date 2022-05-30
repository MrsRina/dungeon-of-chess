#version 330 core

in vec4 varying_material;
in vec4 varying_pos;

uniform sampler2D texture_sampler;
uniform bool contains_texture;

uniform float mx, my;
uniform float w, h;

void main() {
	if (contains_texture) {
		vec4 sampler = texture2D(texture_sampler, vec2(varying_material.x, varying_material.y));

		gl_FragColor = sampler;
	} else {
		gl_FragColor = varying_material;
	}

	vec2 pos = vec2(gl_FragCoord.x, h - gl_FragCoord.y);

	float dx = pos.x - mx;
	float dy = pos.y - my;

	float diff = 255 - clamp(sqrt(dx * dx + dy * dy), 0, 255);
	gl_FragColor.a = diff / 255.0f;
}