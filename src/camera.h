#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "SDL.h"

#define CAMERA_NEAR 0.1f
#define CAMERA_FAR 2000.0f
#define CAMERA_FOV 60.0f

typedef struct Camera Camera;
struct Camera {
	int width;
	int height;

	float fov;
	float z_near;
	float z_far;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float pitch;
	float yaw;
	float sensitivity;
	SDL_bool enabled;
};

/* Initialize a camera at (0, 2, 0) with (0, 1, 0) as up vector, looking down negative z-axis */
void camera_init(Camera* c, int width, int height, float fov, float z_near, float z_far);
glm::mat4 camera_view_matrix(Camera* c);
glm::mat4 camera_projection_matrix(Camera* c);
void camera_translate(Camera* c, const char* direction, float delta_time);
void camera_movement(Camera* c, float delta_time);
void camera_toggle_mouse(Camera* c, SDL_Window* w);
void camera_track1(Camera* c, Uint32 time, Uint32 duration);
void camera_track2(Camera* c, Uint32 time, Uint32 duration);
void camera_track3(Camera* c, Uint32 time, Uint32 duration);
void camera_track4(Camera* c, Uint32 time, Uint32 duration);

#endif