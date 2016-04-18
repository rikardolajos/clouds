#version 450

layout(location = 0) in float height;
layout(location = 1) in float dist;
layout(location = 2) in vec2 pos;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fcolor;

uniform sampler2D terrain_texture;


void main () {

	vec3 fog_color = vec3(0.3, 0.3, 0.7);
	vec3 terrain_color_base = vec3(0.23, 0.38, 0.03);
	vec3 terrain_color_top = vec3(0.4, 0.4, 0.1);

	vec3 fog = fog_color * smoothstep(300, 500, dist) * 1;
	vec3 terrain = terrain_color_base + clamp(height, -0.8, 1.0) * terrain_color_top * texture(terrain_texture, uv * 100).rgb;
	//terrain = vec3(0.0);
	vec3 color = terrain + fog;
	fcolor = vec4(color, smoothstep(580, 330, dist));
	fcolor.rgb *= 1.5;

	/* Grid */
	if (mod(pos.x, 4) < 0.2 || mod(pos.y, 4) < 0.2) {
		//fcolor = vec4(1.0, 1.0, 0.4, 1.0);
	}
}