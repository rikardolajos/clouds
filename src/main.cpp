#include "GL/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "camera.h"
#include "cloud_tiling.h"
#include "cloud_preprocess.h"
#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "log.h"
#include "model.h"
#include "quad.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <chrono>

#define OPENGL_MAJOR_VERSION 4
#define OPENGL_MINOR_VERSION 5

#define FULLSCREEN 0
#define VERTICAL_SYNC 0

#define CLOUD_TILE 0

#if FULLSCREEN
	#define SCREEN_WIDTH 1920
	#define SCREEN_HEIGHT 1600
#else
	#define SCREEN_WIDTH 1280
	#define SCREEN_HEIGHT 720
#endif

/* Main function */
int main(int argc, char** argv)
{
	/* Initialize Log file */
	log_init();

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		log("Error: %s\n", SDL_GetError());
		return 1;
	}
	
	/*Set OpenGL attributes */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);		// Anti-aliasing: 0 = Off, 4 = On
	
	/* Create SDL window */
#if FULLSCREEN
	SDL_Window* window = SDL_CreateWindow("Clouds", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
#else
	SDL_Window* window = SDL_CreateWindow("Clouds", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
#endif

	if (window == NULL) {
		log("Error: %s\n", SDL_GetError());
		return -1;
	}

	/* Create a window icon */
	SDL_Surface *icon = SDL_LoadBMP("./res/window_icon.bmp");
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);

	/* Create OpenGL context */
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		log("Error: %s\n", SDL_GetError());
		return -1;
	}

	/* Use vertical sync */
	if (SDL_GL_SetSwapInterval(VERTICAL_SYNC) != 0) {
		log("Error: %s\n", SDL_GetError());
	}

	/* Start GLEW */
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		log("Error: GLEW failed to initialize.\n");
	}

	/* OpenGL settings */
	log_opengl_clear_errors();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthRange(CAMERA_NEAR, CAMERA_FAR);
	//glEnable(GL_TEXTURE_3D);
	log_opengl_error();

	/* Initialize camera */
	Camera camera;
	camera_init(&camera, SCREEN_WIDTH, SCREEN_HEIGHT, CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);

	/* Initialize shaders */
	log("Loading sky shader...\n");
	Shader sky_shader;
	if (shader_init(&sky_shader, "./res/shaders/sky.vert", "./res/shaders/sky.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading terrain shader...\n");
	Shader terrain_shader;
	if (shader_init(&terrain_shader, "./res/shaders/terrain.vert", "./res/shaders/terrain.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

#if CLOUD_TILE
	log("Loading resolve shader (tile based)...\n");
	Shader resolve_shader;
	if (shader_init(&resolve_shader, "./res/shaders/resolve.vert", "./res/shaders/resolve_tile.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}
#else
	log("Loading resolve shader (noise based)...\n");
	Shader resolve_shader;
	if (shader_init(&resolve_shader, "./res/shaders/resolve.vert", "./res/shaders/resolve_noise.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}
#endif

	log("Loading blur shader...\n");
	Shader blur_shader;
	if (shader_init(&blur_shader, "./res/shaders/blur.vert", "./res/shaders/blur.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading post shader...\n");
	Shader post_shader;
	if (shader_init(&post_shader, "./res/shaders/post.vert", "./res/shaders/post.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	/* Set up framebuffers */
	log("Setting up framebuffers...\n");
	Framebuffer scene_framebuffer;
	framebuffer_scene_init(&scene_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

	Framebuffer cloud_framebuffer;
	framebuffer_cloud_init(&cloud_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

	Framebuffer pingpong_framebuffer[2];
	framebuffer_pingpong_init(pingpong_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

	/* Load models */
	log("\nLoading sky dome...\n");
	Model sky_model;
	if (model_load_obj(&sky_model, "./res/models/sky_uv.obj") != 0) {
		log("Error: Failed to load model in %s at line %d.\n\n", __FILE__, __LINE__);
	}
	sky_model.shader = sky_shader;
	sky_model.scale = glm::vec3(1000.0f, 1000.0f, 1000.0f);
	Model sun;

	log("Loading terrain plane...\n");
	Model terrain_model;
	if (model_load_obj(&terrain_model, "./res/models/terrain_uv.obj") != 0) {
		log("Error: Failed to load model in %s at line %d.\n\n", __FILE__, __LINE__);
	}
	terrain_model.shader = terrain_shader;

	/* Create textures */
	log("\nLoading terrain texture...\n");
	Texture terrain_texture;
	if (texture2D_from_ex5(&terrain_texture, "./res/textures/terrain.ex5") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading 3D cloud texture...\n");
	Texture cloud_texture;
	if (texture3D_from_ex5(&cloud_texture, "./res/textures/noise5.ex5") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading Mie phase texture...\n");
	Texture mie_texture;
	if (texture1D_phase(&mie_texture, "./res/textures/phase.txt") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

#if CLOUD_TILE
	/* Preprocess for the tile based clouds */
	log("\nPreprocessing cloud structure (tile based)...\n");
	Texture cloud_structure;
	Texture cloud_tiles[5];
	cloud_tiling_init(cloud_tiles, &cloud_structure);

	/* Send textures to shaders */
	shader_send_texture3D(resolve_shader, cloud_structure, "cloud_structure");
	shader_send_texture3D(resolve_shader, cloud_tiles[0], "cloud_tile00");
#else
	/* Preprocess the structure of the noise based clouds */
	log("\nPreprocessing cloud structure (noise based)...\n");
	Texture cloud_structure_texture;
	cloud_preprocess(&cloud_structure_texture, cloud_texture);

	/* Send textures to shaders */
	shader_send_texture3D(resolve_shader, cloud_texture, "cloud_texture");
	shader_send_texture3D(resolve_shader, cloud_structure_texture, "cloud_structure");
#endif

	/* Send textures to shaders */
	shader_send_texture2D(terrain_shader, terrain_texture, "terrain_texture");
	shader_send_texture1D(resolve_shader, mie_texture, "mie_texture");

	/* Main loop */
	log("\nStarting main loop.\n");
	bool exit = false;
	float delta_time = 0.0f;
	auto last_frame = std::chrono::high_resolution_clock::now();
	float avrage_timer = 0.0f;
	bool camera_test = false;
	int camera_track = 1;
	Uint64 camera_time = 0;
	Uint64 camera_start = 0;
	Uint64 camera_end = 0;
	Uint64 camera_era = 0;
	Uint64 camera_duration = 20 * SDL_GetPerformanceFrequency();
	Uint64 frame_counter = 0;
	FILE* perf;
	Uint64 prev_count = 0;

	while (!exit) {
		/* Frame time calculation */
		auto current_time = std::chrono::high_resolution_clock::now();
		delta_time = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_frame).count() / 1000.0;
		last_frame = current_time;
		avrage_timer += delta_time;

		if (avrage_timer > 1000) {
			char title[128];
			sprintf(title, "Clouds -- Frame time: %.2f ms, FPS: %.0f", delta_time, 1000.0 / delta_time);
			SDL_SetWindowTitle(window, title);
			avrage_timer = 0;
		}

		/* Event handling */
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				exit = true;
			}

			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					exit = true;;
				}
				if (e.key.keysym.sym == SDLK_r) {
					log("Recompiling active shaders...\n\n");
					shader_recompile();

					/* Send textures to shaders */
#if CLOUD_TILE
					
					shader_send_texture3D(resolve_shader, cloud_structure, "cloud_structure");
					shader_send_texture3D(resolve_shader, cloud_tiles[0], "cloud_tile00");
#else
					shader_send_texture3D(resolve_shader, cloud_texture, "cloud_texture");
					shader_send_texture3D(resolve_shader, cloud_structure_texture, "cloud_structure");
#endif
					shader_send_texture2D(terrain_shader, terrain_texture, "terrain_texture");
					shader_send_texture1D(resolve_shader, mie_texture, "mie_texture");
				}
				if (e.key.keysym.sym == SDLK_F1) {
					if (!camera_test) {
						log("Running performance test...");
						perf = fopen("./performance_data_inv_cpu.txt", "w");
						frame_counter = 0;
						prev_count = SDL_GetPerformanceCounter();
						camera_track = 1;
						camera_start = SDL_GetPerformanceCounter();
						camera_end = camera_start + camera_duration;
						camera_era = camera_start;
						camera_test = true;
					}
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN) {
				if (e.button.button == SDL_BUTTON_LEFT) {
					camera_toggle_mouse(&camera, window);
				}
			}

			if (e.type == SDL_MOUSEMOTION) {
				if (camera.enabled == SDL_TRUE) {
					camera.pitch -= camera.sensitivity * e.motion.yrel;
					camera.yaw += camera.sensitivity * e.motion.xrel;
				}

			}
		}

		///////////////////////////////////////////////////// http://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf
		///////////////////////////////////////////////////// http://freespace.virgin.net/hugo.elias/models/m_clouds.htm
		///////////////////////////////////////////////////// http://www.neilblevins.com/cg_education/procedural_noise/procedural_noise.html
		///////////////////////////////////////////////////// http://wiki.nuaj.net/index.php?title=Clouds
		///////////////////////////////////////////////////// http://www-evasion.imag.fr/Publications/2006/BNL06/
		///////////////////////////////////////////////////// http://learnopengl.com/#!Advanced-Lighting/HDR

		/* Camera movement */
		if (camera_test) {

			camera_time = SDL_GetPerformanceCounter() - camera_start;

			
			double frame_time = (double)((SDL_GetPerformanceCounter() - prev_count) * 1000) / SDL_GetPerformanceFrequency();
			double total_time = (double)((SDL_GetPerformanceCounter() - camera_era) * 1000) / SDL_GetPerformanceFrequency();

			fprintf(perf, "%f %f\n", total_time, frame_time);
			prev_count = SDL_GetPerformanceCounter();

			switch (camera_track) {
			case 1:
				camera_track1(&camera, camera_time, camera_duration);
				break;
			case 2:
				camera_track2(&camera, camera_time, camera_duration);
				break;
			case 3:
				camera_track3(&camera, camera_time, camera_duration);
				break;
			case 4:
				camera_track4(&camera, camera_time, camera_duration);
				break;
			}

			if (camera_time > camera_duration) {
				camera_track++;
				camera_time = 0;
				camera_start = SDL_GetPerformanceCounter();
				camera_end = camera_start + camera_duration;
				double elapsed_time = (double)((camera_start - camera_era) * 1000) / SDL_GetPerformanceFrequency();
				if (camera_track > 4) {
					log("Done\n");
					log("Time passed: %f ms\n", elapsed_time);
					log("Total frames rendered: %" SDL_PRIu64 "\n", frame_counter);
					log("Average frame time: %f ms\n", elapsed_time / frame_counter);
					log("Average frequency: %f f/s\n\n", (double)frame_counter / (elapsed_time / 1000));
					fprintf(perf, "\n");
					fclose(perf);
					camera_test = false;
				}
			}
		} else {
			camera_movement(&camera, delta_time);
		}
		sky_model.position = camera.position;
		sun.position = camera.position + glm::vec3(1000.0f, 1000.0f, 1000.0f);

		/* Calculate view and projection matrices and send them to shaders */
		shader_uniform_mat4(terrain_shader, model_model_matrix(terrain_model), "model");
		shader_uniform_mat4(terrain_shader, camera_view_matrix(&camera), "view");
		shader_uniform_mat4(terrain_shader, camera_projection_matrix(&camera), "proj");

		shader_uniform_mat4(sky_shader, model_model_matrix(sky_model), "model");
		shader_uniform_mat4(sky_shader, camera_view_matrix(&camera), "view");
		shader_uniform_mat4(sky_shader, camera_projection_matrix(&camera), "proj");

		shader_uniform_mat4(resolve_shader, camera_view_matrix(&camera), "view");
		shader_uniform_mat4(resolve_shader, camera_projection_matrix(&camera), "proj");

		shader_uniform_mat4(resolve_shader, glm::inverse(camera_view_matrix(&camera)), "inv_view");
		shader_uniform_mat4(resolve_shader, glm::inverse(camera_projection_matrix(&camera)), "inv_proj");

		/* Send miscellaneous uniforms to shaders */
		shader_uniform_vec3(terrain_shader, camera.position, "camera_pos");

		shader_uniform_vec3(sky_shader, camera.position, "camera_pos");
		shader_uniform_vec3(sky_shader, sun.position, "sun_pos");

		shader_uniform_vec2(resolve_shader, glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), "view_port");
		shader_uniform_vec3(resolve_shader, camera.position, "camera_pos");
		shader_uniform_vec3(resolve_shader, sun.position, "sun_pos");

		/*** OpenGL rendering ***/

		/* Render terrain and sky */
		glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model_render(sky_model);
		model_render(terrain_model);

		/* Render clouds via resolve shader */
		glBindFramebuffer(GL_FRAMEBUFFER, cloud_framebuffer.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_use(resolve_shader);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, scene_framebuffer.color_buffer[0]);
		GLuint diffuse_buffer = glGetUniformLocation(resolve_shader.shader_program, "diffuse_buffer");
		glUniform1i(diffuse_buffer, 10);

		render_quad();

		/* Pinpong buffers for bloom */
		GLboolean horizontal = true, first_iteration = true;
		GLuint amount = 10;
		shader_use(blur_shader);
		for (GLuint i = 0; i < amount; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffer[horizontal].fbo);
			glUniform1i(glGetUniformLocation(blur_shader.shader_program, "horizontal"), horizontal);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? cloud_framebuffer.color_buffer[1] : pingpong_framebuffer[!horizontal].color_buffer[0]);
			GLuint image = glGetUniformLocation(blur_shader.shader_program, "image");
			glUniform1i(image, 5);

			render_quad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}

		/* Render to screen via post shader */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader_use(post_shader);

		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_2D, cloud_framebuffer.color_buffer[0]);
		GLuint hdr_buffer = glGetUniformLocation(post_shader.shader_program, "HDR_buffer");
		glUniform1i(hdr_buffer, 11);

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, pingpong_framebuffer[!horizontal].color_buffer[0]);
		GLuint bloom = glGetUniformLocation(post_shader.shader_program, "bloom_blur");
		glUniform1i(bloom, 12);

		render_quad();

		SDL_GL_SwapWindow(window);

		frame_counter++;
	}

	/* Shutdown functions */
	SDL_Quit();

	return 0;
}