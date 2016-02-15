#version 450

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 text;

layout(location = 0) out float height;
layout(location = 1) out float dist;
layout(location = 2) out vec2 pos;
layout(location = 3) out vec2 uv;

uniform sampler2D terrain_texture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 camera_pos;

void main() {
	dist = length(vert - camera_pos);
	height = texture(terrain_texture, vert.xz * 5).g * 0.4;
	pos = vec2(vert.x, vert.z);
	uv = text;
	gl_Position = proj * view * model * vec4(vert.x, vert.y + height * 3 - 3, vert.z, 1.0);
}