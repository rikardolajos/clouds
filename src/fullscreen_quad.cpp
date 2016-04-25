#include "fullscreen_quad.h"

#include "GL/glew.h"
#include "log.h"
#include "texture.h"

#include <stdlib.h>

/* http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/ */
/* http://learnopengl.com/#!Advanced-Lighting/HDR */
/* http://learnopengl.com/#!Advanced-Lighting/Bloom */

void fs_quad_init(FS_Quad* q, int screen_width, int screen_height, Shader s)
{
	q->screen_width = screen_width;
	q->screen_height = screen_height;
	q->shader = s;

	/* Reserve two texture indices */
	texture_activate(&q->texture);
	texture_activate(&q->texture);
	q->texture.index--;

	/* Create framebuffer */
	glGenFramebuffers(1, &q->framebuffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, q->framebuffer_object);

	/* Create textures to render to */
	glGenTextures(2, q->color_buffers);
	q->texture.object = q->color_buffers[0];

	for (GLuint i = 0; i < 2; i++) {
		/* Send an empty texture to OpenGL and set some filtering */
		glBindTexture(GL_TEXTURE_2D, q->color_buffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, q->screen_width, q->screen_height, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		/* Set color attachment */
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, q->color_buffers[i], 0);
	}
	
	/* Depth buffer */
	glGenRenderbuffers(1, &q->depth_render_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, q->depth_render_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, q->screen_width, q->screen_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, q->depth_render_buffer);

	/* Set the list of draw buffers */
	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, draw_buffers);

	/* Check for errors */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		log("Error: Framebuffer not created!\n");
	}

	/* Create fullscreen quad */
	float quad[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
	
	glGenVertexArrays(1, &q->vertex_array_object);
	glBindVertexArray(q->vertex_array_object);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void fs_quad_pingpong_init(FS_Quad pingpong[2], int screen_width, int screen_height, Shader s)
{
	pingpong[0].screen_width = screen_width;
	pingpong[0].screen_height = screen_height;
	pingpong[0].shader = s;

	pingpong[1].screen_width = screen_width;
	pingpong[1].screen_height = screen_height;
	pingpong[1].shader = s;

	texture_activate(&pingpong[0].texture);
	texture_activate(&pingpong[1].texture);

	glGenFramebuffers(1, &pingpong[0].framebuffer_object);
	glGenTextures(1, &pingpong[0].texture.object);

	glGenFramebuffers(1, &pingpong[1].framebuffer_object);
	glGenTextures(1, &pingpong[1].texture.object);

	for (GLuint i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpong[i].framebuffer_object);
		glBindTexture(GL_TEXTURE_2D, pingpong[i].texture.object);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, pingpong[i].screen_width, pingpong[i].screen_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pingpong[i].texture.object, 0);
	}

	/* Create fullscreen quad */
	float quad[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);

	glGenVertexArrays(1, &pingpong[0].vertex_array_object);
	glBindVertexArray(pingpong[0].vertex_array_object);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

/* http://learnopengl.com/#!Advanced-Lighting/Bloom */
void fs_quad_pingpong_render(FS_Quad pingpong[2], FS_Quad q)
{
	GLboolean horizontal = true;
	GLboolean first_iteration = true;
	GLuint amount = 10;

	glUseProgram(pingpong[0].shader.shader_program);
	
	for (GLuint i = 0; i < amount; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpong[horizontal].framebuffer_object);

		glUniform1i(glGetUniformLocation(pingpong[0].shader.shader_program, "horizontal"), horizontal);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? q.color_buffers[1] : pingpong[!horizontal].texture.object);
		GLuint image = glGetUniformLocation(pingpong[0].shader.shader_program, "image");
		glUniform1i(image, 0);

		glBindVertexArray(pingpong[0].vertex_array_object);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
}

void fs_quad_set_as_render_target(FS_Quad q)
{
	glBindFramebuffer(GL_FRAMEBUFFER, q.framebuffer_object);
	glViewport(0, 0, q.screen_width, q.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void fs_quad_render_to_post(FS_Quad q, FS_Quad post)
{
	glBindFramebuffer(GL_FRAMEBUFFER, post.framebuffer_object);
	glViewport(0, 0, post.screen_width, post.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(q.shader.shader_program);

	glActiveTexture(GL_TEXTURE0 + q.texture.index);
	glBindTexture(GL_TEXTURE_2D, q.texture.object);
	GLuint buffer = glGetUniformLocation(q.shader.shader_program, "diffuse_buffer");
	glUniform1i(buffer, q.texture.index);

	glBindVertexArray(q.vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void fs_quad_render_to_screen(FS_Quad q, FS_Quad pingpong[2])
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, q.screen_width, q.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(q.shader.shader_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, q.texture.object);
	GLuint buffer = glGetUniformLocation(q.shader.shader_program, "HDR_buffer");
	glUniform1i(buffer, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpong[1].texture.object);
	GLuint bloom = glGetUniformLocation(q.shader.shader_program, "bloom_blur");
	glUniform1i(bloom, 1);

	glBindVertexArray(q.vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}