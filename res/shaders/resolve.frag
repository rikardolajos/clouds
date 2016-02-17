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

vec4 cloud_sampling(vec3 v, vec3 color, float delta) {
	vec4 value = vec4(0.0);
	value.rgb = color;

	float p = texture(perlin1, v.xyz / 50).b;
	float w = texture(worley, v.xyz / 150).r;
	value.a = p * w * delta * 0.1;

	return value;
}

// http://www.iquilezles.org/www/articles/terrainmarching/terrainmarching.htm
vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta = 1.0;
	float start = gl_DepthRange.near;
	float end = 1000.0;

	vec4 value = vec4(0.0);
	vec3 cloud_light = vec3(1.0);
	vec3 cloud_dark = vec3(0.95);

	float length_inside = 0.0;
	vec3 inside_start = vec3(0.0);
	vec3 inside_end = vec3(0.0);
	bool inside = false;
	bool was_inside = false;

	for (float t = start; t < end; t += delta) {
		vec3 v = origin + dir * t;
		was_inside = false;

		v.y += 0.06 * t; // Pull down the horizon
		float coverage = smoothstep(0.2, 0.3, pow(texture(perlin1, vec3(v.xz / 500, 0.3)).a, 2.5));
		float underside = smoothstep(cloud_base, cloud_base + 35.0, v.y);
		float height = underside - smoothstep(150.0, 150.0 + 10.0 * coverage, v.y);

		// Set a limit for when we are inside a cloud
		if (height > 0.0) {
			vec4 color = cloud_sampling(v, cloud_light, delta) * coverage * underside;
			value += color;

			// Set start point of new inside episode
			if (!inside) {
				inside = true;
				inside_start = v;
			}
			was_inside = true;
		}

		// If we used to be inside but wasn't this iteration, we are outside
		if (inside && !was_inside) {
			inside = false;
			inside_end = v;
			length_inside += length(inside_end - inside_start);
			break;
		}

		// Adaptive step length
		delta = 0.005 * t;
	}

	length_inside = smoothstep(00, 120, length_inside);
	value.rgb = clamp(value.rgb, vec3(0.0), vec3(1.0));
	value.rgb = mix(value.rgb, cloud_dark, length_inside);

	return value;
}

vec4 cast_ray_cube(vec3 origin, vec3 dir) {
	float delta = 0.5;
	float start = gl_DepthRange.near;
	float end = 400.0;

	vec4 value = vec4(0.0);
	vec3 cloud_light = vec3(1.0, 1.0, 1.0);
	vec3 cloud_dark = vec3(0.1);

	float length_inside = 0.0;
	vec3 inside_start = vec3(0.0);
	vec3 inside_end = vec3(0.0);
	bool inside = false;
	bool was_inside = false;

	vec3 v = vec3(0.0);
	for (float t = start; t < end; t += delta) {
		v = origin + dir * t;
		was_inside = false;

		if (v.y > 50 && v.y < 100) {
			if (v.x > 50 && v.x < 100 && v.z > 50 && v.z < 100) {

				vec4 color = cloud_sampling(v, cloud_light, delta);
				value += color;
				
				// Set a limit for when we are inside a cloud
				if (color.r > 0.0) {
					if (!inside) {
						inside = true;
						inside_start = v;
					}
					was_inside = true;
				}
			}
		}

		// If we used to be inside but wasn't this iteration, we are outside
		if (inside && !was_inside) {
			inside = false;
			inside_end = v;
			length_inside += length(inside_end - inside_start);
			//break;
		}

		//delta = 0.005 * t;
	}

	// If we reached last step before exiting a cloud
	if (inside) {
		inside_end = v;
		length_inside += length(inside_end - inside_start);
	}

	length_inside = smoothstep(10, 200, length_inside);
	value.rgba = clamp(value.rgba, vec4(0.0), vec4(1.0));
	value.rgb = mix(value.rgb, cloud_dark, length_inside);

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

	//vec4 cloud_color = cast_ray_(camera_pos, ray_world);
	vec4 cloud_color = cast_ray_cube(camera_pos, ray_world);

	vec4 diffuse_color = texelFetch(diffuse_buffer, ivec2(gl_FragCoord.xy), 0);
	float depth = pow(texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).x, 128.0);
	//frag_color = diffuse_color;
	//frag_color = diffuse_color + test;
	frag_color.a = 1.0;
	frag_color.rgb = mix(diffuse_color.rgb, cloud_color.rgb, cloud_color.a);
	//frag_color.rgb = vec3(cloud_color.r);

	//frag_color = vec4(vec3(texture(perlin1, vec3(x, y, 0.0)).r), 1.0);
	//frag_color = vec4(vec3(texture(perlin1, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.5) * 3).r) * coverage, 1.0);
	//vec3 p = vec3(texture(perlin1, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.34) * 2).b);
	//vec3 w = vec3(texture(worley, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.87) * 2).r);
	//frag_color = vec4(p * w, 1.0);
}