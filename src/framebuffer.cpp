#include "framebuffer.h"

#include "log.h"

#include "GL/glew.h"

#include <stdlib.h>

void framebuffer_scene_init(Framebuffer* f, int screen_width, int screen_height)
{
	glGenFramebuffers(1, &f->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, f->fbo);

	glGenTextures(1, &f->color_buffer[0]);

	glBindTexture(GL_TEXTURE_2D, f->color_buffer[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->color_buffer[0], 0);

	GLuint depth_rbo;
	glGenRenderbuffers(1, &depth_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

	GLuint scene_attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, scene_attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		log("Error: Framebuffer not created!\n");
	}
}

void framebuffer_cloud_init(Framebuffer* f, int screen_width, int screen_height)
{
	glGenFramebuffers(1, &f->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, f->fbo);

	glGenTextures(2, f->color_buffer);
	for (GLuint i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, f->color_buffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, f->color_buffer[i], 0);
	}

	GLuint cloud_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, cloud_attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		log("Error: Framebuffer not created!\n");
	}
}

void framebuffer_pingpong_init(Framebuffer f[2], int screen_width, int screen_height)
{
	glGenFramebuffers(1, &f[0].fbo);
	glGenTextures(1, f[0].color_buffer);

	glGenFramebuffers(1, &f[1].fbo);
	glGenTextures(1, f[1].color_buffer);

	for (GLuint i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, f[i].fbo);
		glBindTexture(GL_TEXTURE_2D, f[i].color_buffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f[i].color_buffer[0], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			log("Framebuffer not complete!");
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}