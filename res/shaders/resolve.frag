#version 450

layout(location = 0) in vec3 vertex_pos;

out vec4 frag_color;

/////////////////////////////////
uniform sampler3D perlin1;
uniform sampler3D td1;
uniform sampler2D terrain_texture;
/////////////////////////////////

uniform sampler2D diffuse_buffer;
uniform sampler2D depth_buffer;

uniform vec2 view_port;
uniform vec3 camera_pos;
uniform vec3 sun_pos;

uniform mat4 view;
uniform mat4 proj;

float height_stratus(float y) {
	float bottom = 50;
	float top = 200;
	return smoothstep(bottom, bottom + 60, y) - smoothstep(top - 20, top, y);
}

float cloud_sampling(vec3 v, float delta) {

	vec4 texture = texture(td1, v / 300);

	float coverage = smoothstep(0.3, 0.301, pow(texture.g, 2.0));
	float height = height_stratus(v.y);

	return texture.r * coverage * height * delta * 0.1;
}

float cast_shadow_ray(vec3 origin, vec3 dir) {
	float delta = 10.0;
	float end = 50.0;

	vec3 sample_point = vec3(0.0);
	float shadowness = 0.0;

	for (float t = 0.0; t < end; t += delta) {
		sample_point = origin + dir * t;
		shadowness += cloud_sampling(sample_point, delta);
	}

	return smoothstep(0, 50, shadowness);
}	

// http://www.iquilezles.org/www/articles/terrainmarching/terrainmarching.htm
vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta = 1.0;
	float start = gl_DepthRange.near;
	float end = 500.0;

	vec4 value = vec4(0.0);
	vec3 cloud_color = vec3(0.93, 0.93, 0.95);
	vec3 cloud_shade = vec3(0.82, 0.82, 0.9) - 0.2;
	vec3 cloud_dense = vec3(0.98);
	value.rgb = cloud_color;

	/* Test colors */
	//cloud_shade = vec3(1.0, 0.0, 1.0);
	//cloud_dense = vec3(0.0, 1.0, 0.0);

	float length_inside = 0.0;
	vec3 inside_start = vec3(0.0);
	vec3 inside_end = vec3(0.0);
	bool inside = false;
	bool inside_once = false;
	bool inside_last_iteration = false;

	vec3 sample_point = vec3(0.0);

	for (float t = start; t < end; t += delta) {
		sample_point = origin + dir * t;

		/* Don't continue after passing a lot of clouds */
		if (!inside && inside_once && length_inside > 100) {
			//break; // This is stupid and doesn't work!
		}

		/* Pull down the horizon to get a better looking sky */
		sample_point.y += 0.1 * t; 

		inside_last_iteration = false;
		float alpha = cloud_sampling(sample_point, delta);

		if (alpha > 0.00) {
			
			value.a += alpha;

			/* Set start point of new inside episode */
			if (!inside) {
				inside = true;
				inside_start = sample_point;

				if (!inside_once) {
					inside_once = true;
				}
			}

			inside_last_iteration = true;
		}

		/* If we used to be inside but wasn't this iteration, we are outside */
		if (inside && !inside_last_iteration) {
			inside = false;
			inside_end = sample_point;
			length_inside += length(inside_end - inside_start) * value.a;
			if (value.a > 1.0) {
				//break;
			}
		}

		/* Calculate the shadows */
		float shade = cast_shadow_ray(sample_point, normalize(sun_pos - sample_point));
		value.rgb = mix(value.rgb, cloud_shade, shade);

		/* Adaptive step length */
		//delta = 0.02 * t;
	}

	/* If we reached last step before exiting a cloud */
	if (inside) {
		inside_end = sample_point;
		length_inside += length(inside_end - inside_start) * value.a;
	}

	/* Apply shadow by sampling ray from first contact point to the sun */
	//float shade = cast_shadow_ray(first_contact, -normalize(sun_pos - first_contact)) * value.a;

	length_inside = smoothstep(50, 500, length_inside);
	value.rgba = clamp(value.rgba, vec4(0.0), vec4(1.0));
	//value.rgb = mix(value.rgb, cloud_dense, length_inside);
	//value.rgb = mix(value.rgb, cloud_shade, shade);

	return value;
}

void main() {
	/* Calculate the ray */
	// http://antongerdelan.net/opengl/raycasting.html
	float x = 2.0 * gl_FragCoord.x / view_port.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / view_port.y - 1.0;
	vec2 ray_nds = vec2(x, y);
	vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
	vec4 ray_view = inverse(proj) * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 ray_world = (inverse(view) * ray_view).xyz;
	ray_world = normalize(ray_world);

	vec4 cloud_color = cast_ray(camera_pos, ray_world);

	vec4 diffuse_color = texelFetch(diffuse_buffer, ivec2(gl_FragCoord.xy), 0);
	float depth = pow(texelFetch(depth_buffer, ivec2(gl_FragCoord.xy), 0).x, 128.0);

	frag_color.a = 1.0;
	frag_color.rgb = mix(diffuse_color.rgb, cloud_color.rgb, cloud_color.a);
	//frag_color.rgb = vec3(ray_world);

	//frag_color = vec4(vec3(texture(perlin1, vec3(x, y, 0.0)).r), 1.0);
	//frag_color = vec4(vec3(texture(terrain_texture, vec2(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y) * 6)), 1.0);
	//vec3 p = vec3(texture(perlin1, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.32) * 2).b);
	vec3 w = vec3(texture(td1, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.0) * 2).r);
	//frag_color = vec4(w , 1.0);
}