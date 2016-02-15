#version 450

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;

layout(location = 0) smooth out vec3 N;
layout(location = 1) out vec3 S;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 camera_pos;
uniform vec3 sun_pos;

void main() {
	N = norm;
	S = sun_pos - camera_pos;
	gl_Position = proj * view * model * vec4(vert, 1.0);
}