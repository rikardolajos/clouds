#include "cloud_preprocess.h"

#include "glm/glm.hpp"

#include "texture.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* 1 = inside cloud, else 0 */
GLubyte structure(GLubyte pixel)
{
	if (pixel / 255.0 > 0.21) {
		return 1;
	}
	return 0;
}

int cloud_preprocess(Texture* cloud_structure, Texture source)
{
	/* Read the source texture */
	GLubyte* cloud_pixels = (GLubyte*)malloc(source.width * source.height * source.depth * 4 * sizeof(GLubyte));
	glBindTexture(GL_TEXTURE_3D, source.object);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, cloud_pixels);

	/* Set size of structure texture */
	cloud_structure->width = 32;		/* Creates 4x4x4 cubes of original 128x128x128 texture */
	cloud_structure->height = 32;
	cloud_structure->depth = 32;

	/* Pick the red channel and procces it */
	GLubyte* temp = (GLubyte*)malloc(source.width * source.height * source.depth * sizeof(GLubyte));
	for (int i = 0; i < source.width * source.height * source.depth; i++) {
		temp[i] = structure(cloud_pixels[i * 4]);
	}

	GLubyte* new_structure = (GLubyte*)malloc(cloud_structure->width * cloud_structure->height * cloud_structure->depth * sizeof(GLubyte));

	/* Analyze every 4x4x4 block of the original texture */
	int inside = 0;
	for (int i = 0; i < cloud_structure->width; i++) {
		for (int j = 0; j < cloud_structure->height; j++) {
			for (int k = 0; k < cloud_structure->depth; k++) {

				inside = 0;

				for (int u = 0; u < 4; u++) {
					for (int v = 0; v < 4; v++) {
						for (int w = 0; w < 4; w++) {

							int x = i * 4 + u;
							int y = j * 4 + v;
							int z = k * 4 + w;

							inside += temp[x + y * source.height + z * source.height * source.depth];

						}
					}
				}

				if (inside < 8) {
					new_structure[i + j * cloud_structure->height + k * cloud_structure->height * cloud_structure->depth] = 0;
				} else {
					new_structure[i + j * cloud_structure->height + k * cloud_structure->height * cloud_structure->depth] = 255;
				}

			}
		}
	}

	/* Post process -- expanding the structure to reduce artifacts */
	GLubyte* post_pixels = (GLubyte*)calloc(cloud_structure->width * cloud_structure->height * cloud_structure->depth, sizeof(GLubyte));

	for (int i = 0; i < cloud_structure->width; i++) {
		for (int j = 0; j < cloud_structure->height; j++) {
			for (int k = 0; k < cloud_structure->depth; k++) {

				if (new_structure[k + j * cloud_structure->height + i * cloud_structure->height * cloud_structure->depth] == 0) {
					continue;
				}

				for (int ii = -1; ii < 2; ii++) {
					if (i + ii < 0 || i + ii >= cloud_structure->width) continue;
					for (int jj = -1; jj < 2; jj++) {
						if (j + jj < 0 || j + jj >= cloud_structure->height) continue;
						for (int kk = -1; kk < 2; kk++) {
							if (k + kk < 0 || k + kk >= cloud_structure->depth) continue;
							
							post_pixels[(k + kk) + (j + jj) * cloud_structure->height + (i + ii) * cloud_structure->height * cloud_structure->depth] = 255;
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
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	free(cloud_pixels);
	free(temp);
	free(new_structure);
	free(post_pixels);

	return 0;
}