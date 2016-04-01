#version 450

layout(location = 0) in vec3 vertex_pos;

out vec4 frag_color;

/////////////////////////////////
//uniform sampler3D perlin1;
//uniform sampler2D terrain_texture;
/////////////////////////////////

uniform sampler1D mie_texture;
uniform sampler3D cloud_texture;
uniform sampler3D cloud_structure;

uniform sampler2D diffuse_buffer;
uniform sampler2D depth_buffer;

uniform vec2 view_port;
uniform vec3 camera_pos;
uniform vec3 sun_pos;

uniform mat4 view;
uniform mat4 proj;

float PI = 3.1415962;
float PI_r = 0.3183098;

float HG(float costheta) {
	float g = 0.99;
	return 0.25 * PI_r * (1 - pow(g, 2.0)) / pow((1 + pow(g, 2.0) - 2 * g * costheta), 1.5);
}

float Mie(float costheta) {
	float angle = acos(costheta);
	return texture(mie_texture, (PI - angle) * PI_r).r;
}

float phase(vec3 v1, vec3 v2) {
	float costheta = dot(v1, v2) / length(v1) / length(v2);
	return HG(-costheta);
	//return Mie(costheta);
}

float height_stratus(float y, bool low_res) {
	float bottom = 100;
	float top = 250;
	if (low_res) {
		return step(bottom, y) - step(top, y);
	}
	return smoothstep(bottom, bottom + 60, y) - smoothstep(top - 20, top, y);
}

float coverage(float t, float y) {
	/* The lower level must be same as the value in the preprocessors structure function */
	return smoothstep(0.63, 0.65, t); 
}

float cloud_sampling_lowres(vec3 v, float delta) {
	vec4 texture = texture(cloud_structure, v / 400);
	float height = height_stratus(v.y, true);

	return texture.r * height;
}

float cloud_sampling(vec3 v, float delta) {

	vec4 textureA = texture(cloud_texture, v / 400);
	vec4 textureB = texture(cloud_texture, v / 50);

	float coverage = coverage(textureA.r, v.y);
	float height = height_stratus(v.y, false);

	return textureA.g * coverage * height * delta * 0.4 * textureB.b;
}

/******     Kub och sfär    ******/
float cloud_sampling_lowres1(vec3 v, float delta) {
	if (v.x > 0 && v.x < 70 && v.z > 0 && v.z < 70 && v.y > 20 && v.y < 90) {
		return 1.0;
	}
	if (length(v - vec3(-20, 25, 0)) < 30) {
		return 1.0;
	}
	return 0.0;
}

float cloud_sampling1(vec3 v, float delta) {
	if (v.x > 10 && v.x < 60 && v.z > 10 && v.z < 60 && v.y > 30 && v.y < 80) {
		return 0.1;
	}
	if (length(v - vec3(-20, 25, 0)) < 25) {
		return 0.04;
	}
	return 0.0;
}

float cast_scatter_ray(vec3 origin, vec3 dir) {
	float delta = 5.0;
	float end = 50.0;

	vec3 sample_point = vec3(0.0);
	float inside = 0.0;

	float phase = phase(dir, vec3(camera_pos - origin));

	for (float t = 0.0; t < end; t += delta) {
		sample_point = origin + dir * t;
		inside += cloud_sampling(sample_point, delta);
	}

	float beer = exp(-0.2 * inside);

	//float value = clamp(smaoothstep(20, 50, inside), 0.0, 1.0);
	float value = phase + beer;
	return value;
}	

// http://www.iquilezles.org/www/articles/terrainmarching/terrainmarching.htm
vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta_large = 20.0;
	float delta_small = 1.0;
	float start = gl_DepthRange.near;
	float end = 500.0;

	vec4 value = vec4(0.0);
	vec3 cloud_color = vec3(0.93, 0.93, 0.95);
	vec3 cloud_shade = vec3(0.859, 0.847, 0.757) - 0.1;
	vec3 cloud_bright = vec3(0.99, 0.96, 0.95);
	vec3 cloud_dark = vec3(0.671, 0.725, 0.753);
	//vec3 cloud_dense = vec3(0.98);
	value.rgb = cloud_dark;

	/* Test colors */
	//cloud_color = vec3(1.0, 0.5, 0.0);
	//cloud_shade = vec3(1.0, 0.0, 1.0);
	//cloud_dense = vec3(0.0, 1.0, 0.0);
	value.rgb = cloud_dark;

	float length_inside = 0.0;
	//vec3 inside_start = vec3(0.0);
	//vec3 inside_end = vec3(0.0);
	bool inside = false;
	bool looking_for_new_inside = true;
	int points_inside = 0;
	vec3 sample_point = origin;

	float delta = delta_large;
	for (float t = start; t < end; t += delta) {
		sample_point = origin + dir * t;

		/* Stop rays that are going below ground */
		if (sample_point.y < 0.0) {
			break;
		}

		/* Stop rays that already reach full opacity */
		if (value.a > 1.0) {
			break;
		}

		/* Don't start new clouds if we are close to the top */
		//if (inside && points_outside > 20 && sample_point.y > 100) {
		//	break;
		//}

		/* Pull down the horizon to get a better looking sky (does not work with phase function!) */
		//sample_point.y += 0.1 * t; 

		float alpha;
		if (!inside) {
			alpha = cloud_sampling_lowres(sample_point, delta);
			if (alpha > 0.0) {
				inside = true;
			} else {
				looking_for_new_inside = true;
			}
		}

		if (inside) {
			/* Start of a new inside session? */
			if (looking_for_new_inside) {
				/* Move the starting point a large delta backwards */
				t -= delta_large;
				if (t < gl_DepthRange.near) {
					t = gl_DepthRange.near;
				}
				sample_point = origin + dir * t;
				delta = delta_small;
				//inside_start = sample_point;
				looking_for_new_inside = false;
				points_inside = 0;
			}
			
			alpha = cloud_sampling(sample_point, delta); /* Comment this line to see cloud structure */
			value.a += alpha;
			points_inside += 1;
		}

		/* Check next structure block if we are still inside */
		if (inside && points_inside * delta_small > delta_large) {
			alpha = cloud_sampling_lowres(sample_point, delta);
			if (alpha == 0.0) {
				inside = false;
				looking_for_new_inside = true;
				//inside_end = sample_point;
				//length_inside += length(inside_end - inside_start) * value.a;
				delta = delta_large;
			} else {
				points_inside = 0;
			}
		}


		/* Calculate the shadows */
		float energy = cast_scatter_ray(sample_point, normalize(sun_pos - sample_point));
		//value.rgb = mix(value.rgb, cloud_shade, shade);
		//value.rgb = mix(cloud_dark, cloud_bright, energy);
		//value.rgb = vec3(energy);

		/* Adaptive step length */
		//delta_small = 0.02 * t;
	}

	/* If we reached last step before exiting a cloud, we need to close off the sampling */
	//if (inside) {
	//	inside_end = sample_point;
	//	length_inside += length(inside_end - inside_start) * value.a;
	//}

	//length_inside = smoothstep(50, 500, length_inside);
	//value.rgba = clamp(value.rgba, vec4(0.0), vec4(1.0));
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
	vec3 s = vec3(texture(cloud_structure, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.5) * 2).r);
	vec3 t = vec3(texture(cloud_texture, vec3(gl_FragCoord.x / view_port.x, gl_FragCoord.y / view_port.y, 0.5) * 2).b);
	//t = vec3(coverage(t.r, 0.0));
	//frag_color = vec4(s, 1.0);
}