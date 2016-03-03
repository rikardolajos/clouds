#ifndef TEXTURE_H
#define TEXTURE_H

#include "GL/glew.h"

typedef struct Texture Texture;
struct Texture {
	GLuint object;
	GLuint index;
	int width, height, depth;
};

void texture_activate(Texture* t);
int texture2D_from_ex5(Texture* t, const char* file_path);
int texture3D_from_ex5(Texture* t, const char* file_path);

#endif