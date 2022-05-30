#version 330 core

in vec4 varying_material;

void main() {
	gl_FragColor = varying_material;
}