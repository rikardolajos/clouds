#include "cloud_tiling.h"

#include "glm/glm.hpp"

#include "texture.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int cloud_tiling_structure(Texture* cloud_structure)
{
	/* Set up cloud structure */
	cloud_structure->width = 256;
	cloud_structure->height = 256;
	cloud_structure->depth = 256;

	GLubyte* pixels = (GLubyte*)malloc(cloud_structure->width * cloud_structure->height * cloud_structure->depth * sizeof(GLubyte));

	/* Process the pixels */
	for (int i = 0; i < cloud_structure->width; i++) {
		for (int j = 0; j < cloud_structure->height; j++) {
			for (int k = 0; k < cloud_structure->depth; k++) {
				if (j == 1 && i == 127 && k == 127) {
					pixels[k + j * cloud_structure->height + i * cloud_structure->height * cloud_structure->depth] = 1;
				} else {
					pixels[k + j * cloud_structure->height + i * cloud_structure->height * cloud_structure->depth] = 0;
				}
			}
		}
	}

	/* Post process -- expanding the structure to reduce artifacts */
	GLubyte* post_pixels = (GLubyte*)calloc(cloud_structure->width * cloud_structure->height * cloud_structure->depth, sizeof(GLubyte));

	for (int i = 0; i < cloud_structure->width; i++) {
		for (int j = 0; j < cloud_structure->height; j++) {
			for (int k = 0; k < cloud_structure->depth; k++) {

				if (pixels[k + j * cloud_structure->height + i * cloud_structure->height * cloud_structure->depth] == 0) {
					continue;
				}

				for (int ii = -10; ii < 11; ii++) {
					if (i + ii < 0 || i + ii >= cloud_structure->width) continue;
					for (int jj = -10; jj < 11; jj++) {
						if (j + jj < 0 || j + jj >= cloud_structure->height) continue;
						for (int kk = -10; kk < 11; kk++) {
							if (k + kk < 0 || k + kk >= cloud_structure->depth) continue;

							GLubyte p = pixels[(k + kk) + (j + jj) * cloud_structure->height + (i + ii) * cloud_structure->height * cloud_structure->depth];
							if (p == 0) {
								post_pixels[(k + kk) + (j + jj) * cloud_structure->height + (i + ii) * cloud_structure->height * cloud_structure->depth] = 255;
							} else {
								post_pixels[(k + kk) + (j + jj) * cloud_structure->height + (i + ii) * cloud_structure->height * cloud_structure->depth] = p;
							}
						}
					}
				}
			}
		}
	}

	/* Bind the new texture */
	glGenTextures(1, &cloud_structure->object);
	glBindTexture(GL_TEXTURE_3D, cloud_structure->object);

	/* Give it an index */
	texture_activate(cloud_structure);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, cloud_structure->width, cloud_structure->height, cloud_structure->depth, 0, GL_RED, GL_UNSIGNED_BYTE, post_pixels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	free(pixels);
	free(post_pixels);

	return 0;
}

int cloud_tiling_init(Texture cloud_tiles[], Texture* cloud_structure)
{
	/* Set size of structure texture */
	cloud_tiles[0].width = 64;
	cloud_tiles[0].height = 64;
	cloud_tiles[0].depth = 64;
	
	GLubyte* pixels = (GLubyte*)malloc(cloud_tiles[0].width * cloud_tiles[0].height * cloud_tiles[0].depth * sizeof(GLubyte));

	/* Process the pixels */
	for (int i = 0; i < cloud_tiles[0].width; i++) {
		for (int j = 0; j < cloud_tiles[0].height; j++) {
			for (int k = 0; k < cloud_tiles[0].depth; k++) {
				if (i > 32 && j > 32) {
					pixels[k + j * cloud_tiles[0].height + i * cloud_tiles[0].height * cloud_tiles[0].depth] = 255;
				} else {
					pixels[k + j * cloud_tiles[0].height + i * cloud_tiles[0].height * cloud_tiles[0].depth] = 0;
				}
			}
		}
	}

	/* Bind the new texture */
	glGenTextures(1, &cloud_tiles[0].object);
	glBindTexture(GL_TEXTURE_3D, cloud_tiles[0].object);

	/* Give it an index */
	texture_activate(&cloud_tiles[0]);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, cloud_tiles[0].width, cloud_tiles[0].height, cloud_tiles[0].depth, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	free(pixels);

	cloud_tiling_structure(cloud_structure);

	return 0;
}