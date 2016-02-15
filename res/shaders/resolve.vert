#version 450

layout(location = 0) in vec3 vert;

layout(location = 0) out vec3 vertex_pos;

void main() {
	vertex_pos = vert;
	gl_Position = vec4(vert, 1.0);
}