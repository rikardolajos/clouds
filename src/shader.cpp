#include "shader.h"

#include "GL/glew.h"
#include "glm/gtc/type_ptr.hpp"

#include "log.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static Shader* active_shaders[24] = { 0 };
static int active_index = 0;

#define ERROR_LOG 0		/* 0 = print errors to stderr
						   1 = print errors to shader_error_log.txt */

static void shader_info_log(GLuint s)
{
	int max_length = 2048;
	int len = 0;
	char info_log[2048] = { 0 };
	glGetShaderInfoLog(s, max_length, &len, info_log);
	log("--> %s\n", info_log);
}

static void program_info_log(GLuint s)
{
	int max_length = 2048;
	int len = 0;
	char info_log[2048] = { 0 };
	glGetProgramInfoLog(s, max_length, &len, info_log);
	log("--> %s\n", info_log);
}

/* Read from file and store in a string */
static char* read_file(const char* file_path)
{
	char* file_contents;
	long input_file_size;

	FILE* input_file = fopen(file_path, "rb");
	if (input_file == NULL) {
		log("Error: Unable to open file: %s\n", file_path);
		return NULL;
	}
	fseek(input_file, 0, SEEK_END);
	input_file_size = ftell(input_file);
	rewind(input_file);
	file_contents = (char*)malloc((input_file_size + 1) * (sizeof(char)));
	fread(file_contents, sizeof(char), input_file_size, input_file);
	fclose(input_file);
	file_contents[input_file_size] = 0;

	return file_contents;
}

int shader_init(Shader* shader, const char* vertex_shader_path, const char* fragment_shader_path, int recompile)
{
	char* vertex_source = read_file(vertex_shader_path);
	char* fragment_source = read_file(fragment_shader_path);

	if (vertex_source == NULL || fragment_source == NULL) {
		return -1;
	}

	strcpy(shader->path_vertex, vertex_shader_path);
	strcpy(shader->path_fragment, fragment_shader_path);

	if (recompile == 0) {
		active_shaders[active_index++] = shader;
	}

	/* Compile vertex shader */
	log("Compiling: %s\n", vertex_shader_path);
	if (recompile == 0) {
		shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	}	
	glShaderSource(shader->vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(shader->vertex_shader);

	int params = -1;
	glGetShaderiv(shader->vertex_shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		log("Error: Shader %s did not compile.\n", vertex_shader_path);
		shader_info_log(shader->vertex_shader);
		return -1;
	}

	/* Compile fragment shader */
	log("Compiling: %s\n", fragment_shader_path);
	if (recompile == 0) {
		shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	}
	glShaderSource(shader->fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(shader->fragment_shader);

	glGetShaderiv(shader->fragment_shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		log("Error: Shader %s did not compile.\n", fragment_shader_path);
		shader_info_log(shader->fragment_shader);
		return -1;
	}

	/* Link vertex shader and fragment shader */
	log("Linking: %s and %s\n", vertex_shader_path, fragment_shader_path);
	if (recompile == 0) {
		shader->shader_program = glCreateProgram();
	}
	glAttachShader(shader->shader_program, shader->vertex_shader);
	glAttachShader(shader->shader_program, shader->fragment_shader);
	glLinkProgram(shader->shader_program);

	glGetProgramiv(shader->shader_program, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		log("Error: Could not link shader program %s and %s.\n", vertex_shader_path, fragment_shader_path);
		program_info_log(shader->shader_program);
		return -1;
	}
	log("Shader program compiled and linked successfully.\n\n");

	free(vertex_source);
	free(fragment_source);
	
	return 0;
}

void shader_recompile()
{
	for (int i = 0; i < active_index; i++) {
		shader_init(active_shaders[i], active_shaders[i]->path_vertex, active_shaders[i]->path_fragment, 1);
	}
}

void shader_use(Shader shader)
{
	glUseProgram(shader.shader_program);
}

void shader_uniform_mat4(Shader shader, glm::mat4 matrix, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniformMatrix4fv(uni, 1, GL_FALSE, glm::value_ptr(matrix));
}

void shader_uniform_vec2(Shader shader, glm::vec2 vector, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform2fv(uni, 1, glm::value_ptr(vector));
}

void shader_uniform_vec3(Shader shader, glm::vec3 vector, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform3fv(uni, 1, glm::value_ptr(vector));
}

void shader_uniform_1f(Shader shader, float f, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform1f(uni, f);
}

void shader_send_texture1D(Shader shader, Texture t, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform1i(uni, t.index);
	glActiveTexture(GL_TEXTURE0 + t.index);
	glBindTexture(GL_TEXTURE_1D, t.object);
}

void shader_send_texture2D(Shader shader, Texture t, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform1i(uni, t.index);
	glActiveTexture(GL_TEXTURE0 + t.index);
	glBindTexture(GL_TEXTURE_2D, t.object);
}

void shader_send_texture3D(Shader shader, Texture t, const char* name)
{
	glUseProgram(shader.shader_program);
	GLint uni = glGetUniformLocation(shader.shader_program, name);
	glUniform1i(uni, t.index);
	glActiveTexture(GL_TEXTURE0 + t.index);
	glBindTexture(GL_TEXTURE_3D, t.object);
}