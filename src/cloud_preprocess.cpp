#include "cloud_preprocess.h"

#include "texture.h"

#include <math.h>
#include <stdlib.h>

GLubyte structure(GLubyte pixel)
{
	return pixel;
}

int cloud_preprocess(Texture* cloud_structure, Texture source)
{
	/* Read the source texture */
	GLubyte* cloud_pixels = (GLubyte*)malloc(source.width * source.height * source.depth * 4 * sizeof(GLubyte));
	glBindTexture(GL_TEXTURE_3D, source.object);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, cloud_pixels);

	/* Set size of structure texture */
	cloud_structure->width = 16;
	cloud_structure->height = 16;
	cloud_structure->depth = 16;

	/* Process the cloud structure */
	for (int i = 0; i < 256 * 256 * 256 * 4; i += 4) {
		cloud_pixels[i] = structure(cloud_pixels[i]);
	}

	/* Bind the new texture */
	glGenTextures(1, &cloud_structure->object);
	glBindTexture(GL_TEXTURE_3D, cloud_structure->object);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, cloud_structure->width, cloud_structure->height, cloud_structure->depth, 0, GL_RED, GL_UNSIGNED_BYTE, cloud_pixels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	return 0;
}