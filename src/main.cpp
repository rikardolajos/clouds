#include "GL/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "camera.h"
#include "cloud_tiling.h"
#include "cloud_preprocess.h"
#include "fullscreen_quad.h"
#include "log.h"
#include "model.h"
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

#define CLOUD_TILE 1

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
	Shader sky_shader;
	if (shader_init(&sky_shader, "./res/shaders/sky.vert", "./res/shaders/sky.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	Shader terrain_shader;
	if (shader_init(&terrain_shader, "./res/shaders/terrain.vert", "./res/shaders/terrain.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}

#if CLOUD_TILE
	Shader resolve_shader;
	if (shader_init(&resolve_shader, "./res/shaders/resolve.vert", "./res/shaders/resolve_tile.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}
#else
	Shader resolve_shader;
	if (shader_init(&resolve_shader, "./res/shaders/resolve.vert", "./res/shaders/resolve_noise.frag") != 0) {
		log("Error: Failed to initialize shader in %s at line %d.\n\n", __FILE__, __LINE__);
	}
#endif

	/* Initialize fullscreen quad */
	FS_Quad fs_quad;
	fs_quad_init(&fs_quad, SCREEN_WIDTH, SCREEN_HEIGHT, resolve_shader);
	log_opengl_error();

	/* Load models */
	log("Loading sky dome...\n");
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

	log("Loading Perlin noise texture...\n");
	Texture perlin1_texture;
	if (texture3D_from_ex5(&perlin1_texture, "./res/textures/perlin2.ex5") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading 3D cloud texture...\n");
	Texture cloud_texture;
	if (texture3D_from_ex5(&cloud_texture, "./res/textures/3D_texture_2.ex5") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

	log("Loading Mie phase texture...\n");
	Texture mie_texture;
	if (texture1D_phase(&mie_texture, "./res/textures/phase.txt") != 0) {
		log("Error: Failed to load texture in %s at line %d.\n\n", __FILE__, __LINE__);
	}

#if CLOUD_TILE
	/* Preprocess for the tile based clouds */
	log("\nPreprocessing cloud (tile based) structure...\n");
	Texture cloud_structure;
	Texture cloud_tiles[5];
	cloud_tiling_init(cloud_tiles, &cloud_structure);

	/* Send textures to shaders */
	shader_send_texture3D(resolve_shader, cloud_structure, "cloud_structure");
	shader_send_texture3D(resolve_shader, cloud_tiles[0], "cloud_tile00");
#else
	/* Preprocess the structure of the noise based clouds */
	log("\nPreprocessing cloud (noise based) structure...\n");
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
	log("\nStarting main loop.\n\n");
	bool exit = false;
	float delta_time = 0.0f;
	auto last_frame = std::chrono::high_resolution_clock::now();
	float avrage_timer = 0.0f;

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
					shader_send_texture3D(resolve_shader, cloud_structure, "cloud_structure");
					shader_send_texture3D(resolve_shader, cloud_tiles[0], "cloud_tile00");
					shader_send_texture2D(terrain_shader, terrain_texture, "terrain_texture");
					shader_send_texture1D(resolve_shader, mie_texture, "mie_texture");
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
		///////////////////////////////////////////////////// 

		/* Camera movement */
		camera_movement(&camera, delta_time);
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

		/* Send miscellaneous uniforms to shaders */
		shader_uniform_vec3(terrain_shader, camera.position, "camera_pos");

		shader_uniform_vec3(sky_shader, camera.position, "camera_pos");
		shader_uniform_vec3(sky_shader, sun.position, "sun_pos");

		shader_uniform_vec2(resolve_shader, glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), "view_port");
		shader_uniform_vec3(resolve_shader, camera.position, "camera_pos");
		shader_uniform_vec3(resolve_shader, sun.position, "sun_pos");

		

		/* OpenGL rendering */
		fs_quad_set_as_render_target(fs_quad);

		model_render(sky_model);
		model_render(terrain_model);

		fs_quad_render_to_screen(fs_quad);

		SDL_GL_SwapWindow(window);
	}

	/* Shutdown functions */
	SDL_Quit();

	return 0;
}