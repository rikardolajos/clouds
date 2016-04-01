#version 450

layout(location = 0) in vec3 vertex_pos;

out vec4 frag_color;

uniform sampler1D mie_texture;
uniform sampler3D cloud_structure;
uniform sampler3D cloud_tile00;

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

float cloud_sampling_structure(vec3 v, float delta) {
	/* Reposition the cloud first */
	v = (v + vec3(500, -50, 500)) * 0.001;

	vec4 texture = texture(cloud_structure, v);
	return texture.r;
}

float cloud_sampling_tile(float tile, vec3 v, float delta) {
	/* Reposition the tile first */
	v = (v + vec3(500, -50, 500)) * 0.256;

	if (tile * 255 == 1) {
		return texture(cloud_tile00, v).r * delta;
	}

	if (tile * 255 == 255) {
		/* This is the safety tile that surrounds all non-zero tiles */
		//return 0.08 * delta;
	}

	return 0;
}

float cast_scatter_ray(vec3 origin, vec3 dir) {
	float delta = 5.0;
	float end = 50.0;

	vec3 sample_point = vec3(0.0);
	float inside = 0.0;

	float phase = phase(dir, vec3(camera_pos - origin));

	for (float t = 0.0; t < end; t += delta) {
		sample_point = origin + dir * t;
		//inside += cloud_sampling(sample_point, delta);
	}

	float beer = exp(-0.2 * inside);

	float value = phase + beer;
	return value;
}	

// http://www.iquilezles.org/www/articles/terrainmarching/terrainmarching.htm
vec4 cast_ray(vec3 origin, vec3 dir) {
	float delta_large = 3.9;
	float delta_small = 0.1;
	float start = gl_DepthRange.near;
	float end = 500.0;

	vec4 value = vec4(0.0);
	vec3 cloud_color = vec3(0.93, 0.93, 0.95);
	vec3 cloud_shade = vec3(0.859, 0.847, 0.757) - 0.1;
	vec3 cloud_bright = vec3(0.99, 0.96, 0.95);
	vec3 cloud_dark = vec3(0.671, 0.725, 0.753);
	value.rgb = cloud_dark;

	bool inside = false;
	bool looking_for_new_tile = true;
	int points_inside = 0;
	vec3 sample_point = origin;

	float tile;
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

		float alpha;
		if (!inside) {
			tile = cloud_sampling_structure(sample_point, delta);
			if (tile > 0.0) {
				inside = true;
			} else {
				looking_for_new_tile = true;
			}
		}

		if (inside) {
			/* Start of a new tile? */
			if (looking_for_new_tile) {
				/* Move the starting point a large delta backwards */
				t -= delta_large;
				if (t < gl_DepthRange.near) {
					t = gl_DepthRange.near;
				}
				sample_point = origin + dir * t;
				delta = delta_small;
			
				looking_for_new_tile = false;
				points_inside = 0;
			}
			
			alpha = cloud_sampling_tile(tile, sample_point, delta);
			value.a += alpha;
			points_inside += 1;
		}

		/* Check next structure block if we are still inside */
		if (inside && points_inside * delta_small > delta_large) {
			tile = cloud_sampling_structure(sample_point, delta);
			if (tile == 0.0) {
				inside = false;
				looking_for_new_tile = true;

				delta = delta_large;
			} else {
				points_inside = 0;
			}
		}

		/* Calculate the shadows */
		float energy = cast_scatter_ray(sample_point, normalize(sun_pos - sample_point));
		//value.rgb = mix(cloud_dark, cloud_bright, energy);
	}

	value.rgba = clamp(value.rgba, vec4(0.0), vec4(1.0));

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
}