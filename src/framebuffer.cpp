#include "framebuffer.h"

#include "GL/glew.h"
#include "log.h"

void framebuffer_init(int screen_width, int screen_height)
{
	///* Create framebuffer */
	//glGenFramebuffers(1, &q->framebuffer_object);
	//glBindFramebuffer(GL_FRAMEBUFFER, q->framebuffer_object);

	///* Create textures to render to */
	//glGenTextures(2, q->color_buffers);
	//q->texture.object = q->color_buffers[0];

	//for (GLuint i = 0; i < 2; i++) {
	//	/* Send an empty texture to OpenGL and set some filtering */
	//	glBindTexture(GL_TEXTURE_2D, q->color_buffers[i]);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, q->screen_width, q->screen_height, 0, GL_RGBA, GL_FLOAT, 0);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//	/* Set color attachment */
	//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, q->color_buffers[i], 0);
	//}

	///* Depth buffer */
	//glGenRenderbuffers(1, &q->depth_render_buffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, q->depth_render_buffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, q->screen_width, q->screen_height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, q->depth_render_buffer);

	///* Set the list of draw buffers */
	//GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	//glDrawBuffers(2, draw_buffers);

	///* Check for errors */
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	//	log("Error: Framebuffer not created!\n");
	//}
}