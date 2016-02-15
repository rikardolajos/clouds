#ifndef MODEL_H
#define MODEL_H

#include "GL/glew.h"
#include "shader.h"
#include "glm/glm.hpp"

typedef struct Model Model;
struct Model {
	Shader shader;
	GLuint vertex_array_object;
	int triangles;
	
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

int model_load_obj(Model* m, const char* file_path);
void model_render(Model m);
glm::mat4 model_model_matrix(Model m);

#endif