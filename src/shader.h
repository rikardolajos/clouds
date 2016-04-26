#ifndef SHADER_H
#define SHADER_H

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "texture.h"

typedef struct Shader Shader;
struct Shader {
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint shader_program;
	char path_vertex[256];
	char path_fragment[256];
};


int shader_init(Shader* shader, const char* vertex_shader_path, const char* fragment_shader_path, int recompile = 0);
void shader_recompile();
void shader_use(Shader shader);
void shader_uniform_mat4(Shader shader, glm::mat4 matrix, const char* name);
void shader_uniform_vec2(Shader shader, glm::vec2 vector, const char* name);
void shader_uniform_vec3(Shader shader, glm::vec3 vector, const char* name);
void shader_uniform_1f(Shader shader, float f, const char* name);
void shader_send_texture1D(Shader shader, Texture t, const char* name);
void shader_send_texture2D(Shader shader, Texture t, const char* name);
void shader_send_texture3D(Shader shader, Texture t, const char* name);

#endif