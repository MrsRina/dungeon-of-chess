#version 330 core

in vec4 attribute_pos;
in vec4 attribute_material;

out vec4 varying_material;
uniform mat4 matrix;

void main() {
	gl_Position = matrix * attribute_pos;
	varying_material = attribute_material;
}