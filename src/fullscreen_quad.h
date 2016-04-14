#ifndef FULLSCREEN_QUAD_H
#define FULLSCREEN_QUAD_H

#include "shader.h"

typedef struct FS_Quad FS_Quad;
struct FS_Quad {
	GLuint framebuffer_object;
	GLuint vertex_array_object;
	GLuint depth_render_buffer;

	GLuint diffuse_texture;
	GLuint depth_texture;

	int screen_width;
	int screen_height;

	Shader shader;
};

void fs_quad_init(FS_Quad* q, int screen_width, int screen_height, Shader s);
void fs_quad_init_HDR(FS_Quad* q, int screen_width, int screen_height, Shader s);
void fs_quad_set_as_render_target(FS_Quad q);
void fs_quad_render_to_screen(FS_Quad q);


#endif