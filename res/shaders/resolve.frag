#version 450

layout(location = 0) in vec3 vertex_pos;

out vec4 frag_color;

/////////////////////////////////
uniform sampler3D perlin1;
uniform sampler3D worley;
uniform sampler2D terrain_texture;
/////////////////////////////////

uniform sampler2D diffuse_buffer;
uniform sampler2D depth_buffer;

uniform vec2 view_port;
uniform vec3 camera_pos;
uniform vec3 camera_front;
uniform vec3 camera_up;

uniform mat4 view;
uniform mat4 proj;

uniform float cloud_base;
uniform float cloud_top;

vec4 cloud_sampling(vec3 v, float delta) {
	vec4 value = vec4(0.0);

	value += vec4(0.1, 0.1, 0.1, 1.0) * vec4(vec3(texture(perlin1, v.xyz / 50).a), 1.0) * delta / 2.0;
	//value += vec4(0.1, 0.1, 0.11, 1.0) * vec4(vec3(texture(perlin1, v.xyz / 120).g), 1.0) * 0.07;

	return value;
}

// http://www.iquilezles.org/www/articles/terrainmarching/terrainmarching.htm
vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta = 1.0;
	float start = gl_DepthRange.near;
	float end = 1000.0;

	vec4 value = vec4(0.0);

	for (float t = start; t < end; t += delta) {
		vec3 v = origin + dir * t;
		v.y += 0.1 * t; // Pull down the horizon
		float coverage = smoothstep(0.2, 0.3, pow(texture(perlin1, vec3(v.xz / 500, 0.5)).a, 2));
		float underside = smoothstep(cloud_base, cloud_base + 35.0, v.y);
		float height = underside - smoothstep(150.0, 150.0 + 10.0 * coverage, v.y);
		if (height > 0.0) {
			value += cloud_sampling(v, delta) * coverage * underside;
		}
		delta = 0.005 * t;
	}

	return value;
}

vec4 cast_ray_cube(vec3 origin, vec3 dir) {
	float delta = 0.7;
	float start = gl_DepthRange.near;
	float end = 500.0;

	vec4 value = vec4(0.0);

	for (float t = start; t < end; t += delta) {
		vec3 v = origin + dir * t;

		if (v.y > 50 && v.y < 100) {
			if (v.x > 50 && v.x < 100 && v.z > 50 && v.z < 100) {
				value += cloud_sampling(v, delta);
			}
		}

		//delta = 0.5 * t;
	}

	return value;
}

void main() {
	/* Calculate the ray */
	// http://antongerdelan.net/opengl/raycasting.html
	float x = 2.0 * gl_FragCoord.x / view_port.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / view_port.y - 1.0;
	float z = -1.0;
	vec3 p_nds = vec3(x, y, z);
	vec4 p_clip = vec4(p_nds.xyz, 1.0);
	vec4 p_view = inverse(proj) * p_clip;
	p_view = vec4(p_view.xy, -1.0, 1.0);
	vec4 p_world = (inverse(view) * p_view);
	p_world /= p_world.w;
	vec3 ray_world = normalize(p_world.xyz - camera_pos);

	vec4 cloud_color = cast_ray(camera_pos, ray_world);
	//vec4 test =  cast_ray_cube(camera_pos, ray_world);

	vec4 diffuse_color = texelFetch(diffuse_buffer, ivec2(gl_FragCoord.xy), 0);
	float depth = pow(texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).x, 128.0);
	frag_color = diffuse_color;
	//frag_color = diffuse_color + test;
	frag_color = diffuse_color + cloud_color;
	//frag_color = vec4(vec3(texture(perlin1, vec3(x, y, 0.0)).r), 1.0);
	//frag_color = vec4(vec3(texture(perlin1, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.5) * 3).r) * coverage, 1.0);
	frag_color = vec4(vec3(texture(worley, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.85) * 4).r), 1.0);
}