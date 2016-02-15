#include "model.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int model_load_obj(Model* m, const char* file_path)
{
	FILE* fp = fopen(file_path, "r");

	if (fp == NULL) {
		return -1;
	}

	glm::vec3* vertices = NULL;
	int vertices_index = 0;
	float* vertex_buffer = NULL;
	int vertex_buffer_size = 0;

	glm::vec3* normals = NULL;
	int normals_index = 0;
	float* normals_buffer = NULL;
	int normals_buffer_size = 0;

	glm::vec2* texture_coordinates = NULL;
	int texture_coordinates_index = 0;
	float* texture_buffer = NULL;
	int texture_buffer_size = 0;

	char buffer[128] = { 0 };
	char id[8] = { 0 };

	while (fgets(buffer, sizeof(buffer), fp)) {

		sscanf(buffer, "%s ", id);

		if (strcmp("v", id) == 0) {
			/* Vertex */

			float x, y, z;
			sscanf(buffer, "%*s %f %f %f", &x, &y, &z);
			glm::vec3* tmp = (glm::vec3*)realloc(vertices, (1 + vertices_index) * sizeof *vertices);
			if (tmp != NULL) {
				vertices = tmp;
				tmp = NULL;
			} else {
				free(vertices);
				return -1;
			}

			vertices[vertices_index].x = x;
			vertices[vertices_index].y = y;
			vertices[vertices_index].z = z;

			vertices_index++;

		} else if (strcmp("vn", id) == 0) {
			/* Normal */

			float x, y, z;
			sscanf(buffer, "%*s %f %f %f", &x, &y, &z);
			glm::vec3* tmp = (glm::vec3*)realloc(normals, (1 + normals_index) * sizeof *normals);
			if (tmp != NULL) {
				normals = tmp;
				tmp = NULL;
			} else {
				free(normals);
				return -1;
			}

			normals[normals_index].x = x;
			normals[normals_index].y = y;
			normals[normals_index].z = z;

			normals_index++;

		} else if (strcmp("vt", id) == 0) {
			/* Texture coordinate */

			float x, y, z;
			sscanf(buffer, "%*s %f %f %f", &x, &y, &z);
			glm::vec2* tmp = (glm::vec2*)realloc(texture_coordinates, (1 + texture_coordinates_index) * sizeof *texture_coordinates);
			if (tmp != NULL) {
				texture_coordinates = tmp;
				tmp = NULL;
			} else {
				free(texture_coordinates);
				return -1;
			}

			texture_coordinates[texture_coordinates_index].x = x;
			texture_coordinates[texture_coordinates_index].y = y;

			texture_coordinates_index++;

		} else if (strcmp("f", id) == 0) {
			/* Face */

			int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
			sscanf(buffer, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1,
													         &v2, &vt2, &vn2,
													         &v3, &vt3, &vn3);

			/* New vertex */
			int i = (9 + vertex_buffer_size) * sizeof *vertex_buffer;
			float* tmp1 = (float*)realloc(vertex_buffer, (9 + vertex_buffer_size) * sizeof *vertex_buffer);
			if (tmp1 != NULL) {
				vertex_buffer = tmp1;
				tmp1 = NULL;
			} else {
				free(vertex_buffer);
				return -1;
			}
			vertex_buffer[vertex_buffer_size + 0] = vertices[v1 - 1].x;
			vertex_buffer[vertex_buffer_size + 1] = vertices[v1 - 1].y;
			vertex_buffer[vertex_buffer_size + 2] = vertices[v1 - 1].z;
			vertex_buffer[vertex_buffer_size + 3] = vertices[v2 - 1].x;
			vertex_buffer[vertex_buffer_size + 4] = vertices[v2 - 1].y;
			vertex_buffer[vertex_buffer_size + 5] = vertices[v2 - 1].z;
			vertex_buffer[vertex_buffer_size + 6] = vertices[v3 - 1].x;
			vertex_buffer[vertex_buffer_size + 7] = vertices[v3 - 1].y;
			vertex_buffer[vertex_buffer_size + 8] = vertices[v3 - 1].z;
			vertex_buffer_size += 9;

			/* New normal */
			float* tmp2 = (float*)realloc(normals_buffer, (9 + normals_buffer_size) * sizeof *normals_buffer);
			if (tmp2 != NULL) {
				normals_buffer = tmp2;
				tmp2 = NULL;
			} else {
				free(normals_buffer);
				return -1;
			}
			normals_buffer[normals_buffer_size + 0] = normals[vn1 - 1].x;
			normals_buffer[normals_buffer_size + 1] = normals[vn1 - 1].y;
			normals_buffer[normals_buffer_size + 2] = normals[vn1 - 1].z;
			normals_buffer[normals_buffer_size + 3] = normals[vn2 - 1].x;
			normals_buffer[normals_buffer_size + 4] = normals[vn2 - 1].y;
			normals_buffer[normals_buffer_size + 5] = normals[vn2 - 1].z;
			normals_buffer[normals_buffer_size + 6] = normals[vn3 - 1].x;
			normals_buffer[normals_buffer_size + 7] = normals[vn3 - 1].y;
			normals_buffer[normals_buffer_size + 8] = normals[vn3 - 1].z;
			normals_buffer_size += 9;

			/* New texture coordinate */
			float* tmp3 = (float*)realloc(texture_buffer, (6 + texture_buffer_size) * sizeof *texture_buffer);
			if (tmp3 != NULL) {
				texture_buffer = tmp3;
				tmp3 = NULL;
			} else {
				free(texture_buffer);
				return -1;
			}
			texture_buffer[texture_buffer_size + 0] = texture_coordinates[vt1 - 1].x;
			texture_buffer[texture_buffer_size + 1] = texture_coordinates[vt1 - 1].y;
			texture_buffer[texture_buffer_size + 2] = texture_coordinates[vt2 - 1].x;
			texture_buffer[texture_buffer_size + 3] = texture_coordinates[vt2 - 1].y;
			texture_buffer[texture_buffer_size + 4] = texture_coordinates[vt3 - 1].x;
			texture_buffer[texture_buffer_size + 5] = texture_coordinates[vt3 - 1].y;
			texture_buffer_size += 6;

		} else if (strcmp("#", id) == 0) {
			/* Comment -- Do nothing */
		}
	}

	fclose(fp);

	m->triangles = vertex_buffer_size;

	GLuint vert = 0;
	glGenBuffers(1, &vert);
	glBindBuffer(GL_ARRAY_BUFFER, vert);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size * sizeof(float), vertex_buffer, GL_STATIC_DRAW);

	GLuint norm = 0;
	glGenBuffers(1, &norm);
	glBindBuffer(GL_ARRAY_BUFFER, norm);
	glBufferData(GL_ARRAY_BUFFER, normals_buffer_size * sizeof(float), normals_buffer, GL_STATIC_DRAW);

	GLuint text = 0;
	glGenBuffers(1, &text);
	glBindBuffer(GL_ARRAY_BUFFER, text);
	glBufferData(GL_ARRAY_BUFFER, texture_buffer_size * sizeof(float), texture_buffer, GL_STATIC_DRAW);

	glGenVertexArrays(1, &m->vertex_array_object);
	glBindVertexArray(m->vertex_array_object);
	glBindBuffer(GL_ARRAY_BUFFER, vert);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, norm);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, text);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	m->position = glm::vec3(1.0f);
	m->rotation = glm::vec3(0.0f);
	m->scale = glm::vec3(1.0f);

	return 0;
}

void model_render(Model m)
{
	// Borde kanske använda index draw istället
	glUseProgram(m.shader.shader_program);
	glBindVertexArray(m.vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, m.triangles);
}


glm::mat4 model_model_matrix(Model m)
{
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), m.scale);
	glm::mat4 rotation_x = glm::rotate(glm::mat4(1.0f), m.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotation_y = glm::rotate(glm::mat4(1.0f), m.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation_z = glm::rotate(glm::mat4(1.0f), m.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), m.position);

	return translation * rotation_x * rotation_y * rotation_z * scale;
}