#include "cloud_tiling.h"

#include "glm/glm.hpp"

#include "texture.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int cloud_tiling_init(Texture* cloud_tile00)
{
	/* Set size of structure texture */
	cloud_tile00->width = 64;
	cloud_tile00->height = 64;
	cloud_tile00->depth = 64;
	
	GLubyte* pixels = (GLubyte*)malloc(cloud_tile00->width * cloud_tile00->height * cloud_tile00->depth * sizeof(GLubyte));

	/* Process the pixels */
	for (int i = 0; i < cloud_tile00->width; i++) {
		for (int j = 0; j < cloud_tile00->height; j++) {
			for (int k = 0; k < cloud_tile00->depth; k++) {
				if (j > 32) {
					pixels[k + j * cloud_tile00->height + i * cloud_tile00->height * cloud_tile00->depth] = 255;
				}
				pixels[k + j * cloud_tile00->height + i * cloud_tile00->height * cloud_tile00->depth] = 0;
			}
		}
	}

	/* Bind the new texture */
	glGenTextures(1, &cloud_tile00->object);
	glBindTexture(GL_TEXTURE_3D, cloud_tile00->object);

	/* Give it an index */
	texture_activate(cloud_tile00);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, cloud_tile00->width, cloud_tile00->height, cloud_tile00->depth, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	free(pixels);

	return 0;
}