#version 330 core

in vec4 varying_material;

uniform sampler2D texture_sampler;
uniform bool contains_texture;

void main() {
	if (contains_texture) {
		vec4 the_sample = texture2D(texture_sampler, vec2(varying_material.x, varying_material.y));
		gl_FragColor = the_sample;
	} else {
		gl_FragColor = varying_material;
	}
}