#include "texture.h"

#include "GL/glew.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int active_textures = 2;		/* The fullscreen-quad uses two textures */


int texture2D_from_ex5(Texture* t, const char* file_path)
{
	FILE* fp = fopen(file_path, "r");

	if (fp == NULL) {
		return -1;
	}

	t->index = active_textures++;

	/* Read header */
	fscanf(fp, "%d", &t->width);
	fscanf(fp, "%d", &t->height);

	/* Create image */
	uint8_t* data = (uint8_t*)calloc(t->width * t->height * 4, sizeof data);

	/* Read image*/
	int i = 0;
	while (!feof(fp)) {
		uint32_t pixel;
		fscanf(fp, "%d", &pixel);
		data[i++] = pixel >> 24;
		data[i++] = pixel >> 16;
		data[i++] = pixel >> 8;
		data[i++] = pixel >> 0;
	}

	glGenTextures(1, &t->object);
	glBindTexture(GL_TEXTURE_2D, t->object);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return 0;
}

int texture3D_from_ex5(Texture* t, const char* file_path)
{
	FILE* fp = fopen(file_path, "r");

	if (fp == NULL) {
		return -1;
	}

	t->index = active_textures++;

	/* Read header */
	fscanf(fp, "%d", &t->width);
	fscanf(fp, "%d", &t->height);
	fscanf(fp, "%d", &t->depth);

	/* Create image */
	uint8_t* data = (uint8_t*)calloc(t->width * t->height * t->depth * 4, sizeof data);

	/* Read image*/
	int i = 0;
	while (!feof(fp)) {
		uint32_t pixel;
		fscanf(fp, "%d", &pixel);
		data[i++] = pixel >> 24;
		data[i++] = pixel >> 16;
		data[i++] = pixel >> 8;
		data[i++] = pixel >> 0;
	}

	glGenTextures(1, &t->object);
	glBindTexture(GL_TEXTURE_3D, t->object);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, t->width, t->height, t->depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	return 0;
}