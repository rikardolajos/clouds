#ifndef FULLSCREEN_QUAD_H
#define FULLSCREEN_QUAD_H

#include "shader.h"

typedef struct FS_Quad FS_Quad;
struct FS_Quad {
	GLuint framebuffer_object;
	GLuint vertex_array_object;
	GLuint depth_render_buffer;

	int screen_width;
	int screen_height;

	Shader shader;
	Texture texture;
};

void fs_quad_init(FS_Quad* q, int screen_width, int screen_height, Shader s);
void fs_quad_set_as_render_target(FS_Quad q);
void fs_quad_render_to_post(FS_Quad q, FS_Quad post);
void fs_quad_render_to_screen(FS_Quad q);


#endif