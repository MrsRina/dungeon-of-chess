#version 330 core

out vec4 fragment_material_color;
in vec4 material_color;

void main() {
    fragment_material_color = material_color;
}