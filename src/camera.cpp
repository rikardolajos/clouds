#include "camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL.h"

#include <string.h>

#define CAMERA_SPEED 0.02f
#define SPRINT 2
#define MOUSE_SENSITIVITY 0.05f

void camera_init(Camera* c, int width, int height, float fov, float z_near, float z_far)
{
	c->width = width;
	c->height = height;

	c->fov = fov;
	c->z_near = z_near;
	c->z_far = z_far;

	c->position = glm::vec3(0.0f, 2.0f, 0.0f);
	c->front = glm::vec3(0.0f, 0.0f, -1.0f);
	c->up = glm::vec3(0.0f, 1.0f, 0.0f);

	c->pitch = 0.0f;
	c->yaw = -90.0f;
	c->sensitivity = MOUSE_SENSITIVITY;
	c->enabled = SDL_FALSE;
}

glm::mat4 camera_view_matrix(Camera* c)
{
	return glm::lookAt(c->position, c->position + c->front, c->up);
}

glm::mat4 camera_projection_matrix(Camera* c)
{
	return glm::perspective(glm::radians(c->fov), (float)c->width / (float)c->height, c->z_near, c->z_far);
}

void camera_translate(Camera* c, const char* direction, float delta_time)
{
	float camera_speed = CAMERA_SPEED * delta_time;

	if (strcmp(direction, "forward") == 0) {
		c->position += camera_speed * c->front;
	} else if (strcmp(direction, "backward") == 0) {
		c->position -= camera_speed * c->front;
	} else if (strcmp(direction, "left") == 0) {
		c->position -= glm::normalize(glm::cross(c->front, c->up)) * camera_speed;
	} else if (strcmp(direction, "right") == 0) {
		c->position += glm::normalize(glm::cross(c->front, c->up)) * camera_speed;
	} else if (strcmp(direction, "up") == 0) {
		c->position += camera_speed * c->up;
	} else if (strcmp(direction, "down") == 0) {
		c->position -= camera_speed * c->up;
	}
}

void camera_movement(Camera* c, float delta_time)
{
	if (c->pitch > 89.5f)
		c->pitch = 89.5f;
	if (c->pitch < -89.5f)
		c->pitch = -89.5f;
	if (c->position.y < 3.0)
		c->position.y = 3.0;


	c->front.x = cos(glm::radians(c->pitch)) * cos(glm::radians(c->yaw));
	c->front.y = sin(glm::radians(c->pitch));
	c->front.z = cos(glm::radians(c->pitch)) * sin(glm::radians(c->yaw));

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LSHIFT])
		delta_time *= SPRINT;
	if (state[SDL_SCANCODE_W])
		camera_translate(c, "forward", delta_time);
	if (state[SDL_SCANCODE_S])
		camera_translate(c, "backward", delta_time);
	if (state[SDL_SCANCODE_A])
		camera_translate(c, "left", delta_time);
	if (state[SDL_SCANCODE_D])
		camera_translate(c, "right", delta_time);
	if (state[SDL_SCANCODE_SPACE])
		camera_translate(c, "up", delta_time);
	if (state[SDL_SCANCODE_LCTRL])
		camera_translate(c, "down", delta_time);
}

/* Switch between mouse look and mouse control */
void camera_toggle_mouse(Camera* c, SDL_Window* w)
{
	if (c->enabled) {
		c->enabled = SDL_FALSE;
	} else {
		c->enabled = SDL_TRUE;
	}
	SDL_SetRelativeMouseMode(c->enabled);
	if (c->enabled == SDL_FALSE)
		SDL_WarpMouseInWindow(w, c->width / 2, c->height / 2);
}

/* Defines a camera track using a Bézier curve */
void camera_track1(Camera* c, Uint32 time, Uint32 duration)
{
	float t = (float)time / duration;
	glm::vec3 p0 = glm::vec3(0.0f, 5.0f, 0.0f);
	glm::vec3 p1 = glm::vec3(70.0f, 300.0f, -20.0f);
	glm::vec3 p2 = glm::vec3(150.0f, 200.0f, 220.0f);
	glm::vec3 p3 = glm::vec3(230.0f, 50.0f, 270.0f);

	c->pitch = 20.0f;
	c->yaw = 50.0f;
	c->position = powf((1 - t), 3) * p0 + 3 * powf((1 - t), 2) * t * p1 + 3 * (1 - t) * powf(t, 2) * p2 + powf(t, 3) * p3;

	c->front.x = cos(glm::radians(c->pitch)) * cos(glm::radians(c->yaw));
	c->front.y = sin(glm::radians(c->pitch));
	c->front.z = cos(glm::radians(c->pitch)) * sin(glm::radians(c->yaw));
}

void camera_track2(Camera* c, Uint32 time, Uint32 duration)
{
	float t = (float)time / duration;
	glm::vec3 p0 = glm::vec3(250.0f, 200.0f, -220.0f);
	glm::vec3 p1 = glm::vec3(250.0f, 250.0f, 50.0f);
	glm::vec3 p2 = glm::vec3(200.0f, 100.0f, 120.0f);
	glm::vec3 p3 = glm::vec3(100.0f, 200.0f, 220.0f);
	

	c->pitch = -5.0f;
	c->yaw = 90.0f;
	c->position = powf((1 - t), 3) * p0 + 3 * powf((1 - t), 2) * t * p1 + 3 * (1 - t) * powf(t, 2) * p2 + powf(t, 3) * p3;

	c->front.x = cos(glm::radians(c->pitch)) * cos(glm::radians(c->yaw));
	c->front.y = sin(glm::radians(c->pitch));
	c->front.z = cos(glm::radians(c->pitch)) * sin(glm::radians(c->yaw));
}

void camera_track3(Camera* c, Uint32 time, Uint32 duration)
{
	float t = (float)time / duration;
	glm::vec3 p0 = glm::vec3(350.0f, 350.0f, 350.0f);
	glm::vec3 p1 = glm::vec3(250.0f, 250.0f, 100.0f);
	glm::vec3 p2 = glm::vec3(100.0f, 50.0f, 120.0f);
	glm::vec3 p3 = glm::vec3(10.0f, 250.0f, 20.0f);


	c->pitch = -30.0f;
	c->yaw = -170.0f;
	c->position = powf((1 - t), 3) * p0 + 3 * powf((1 - t), 2) * t * p1 + 3 * (1 - t) * powf(t, 2) * p2 + powf(t, 3) * p3;

	c->front.x = cos(glm::radians(c->pitch)) * cos(glm::radians(c->yaw));
	c->front.y = sin(glm::radians(c->pitch));
	c->front.z = cos(glm::radians(c->pitch)) * sin(glm::radians(c->yaw));
}

void camera_track4(Camera* c, Uint32 time, Uint32 duration)
{
	float t = (float)time / duration;
	glm::vec3 p0 = glm::vec3(-100.0f, 0.0f, -350.0f);
	glm::vec3 p1 = glm::vec3(0.0f, 0.0f, -100.0f);
	glm::vec3 p2 = glm::vec3(100.0f, 50.0f, 20.0f);
	glm::vec3 p3 = glm::vec3(200.0f, 300.0f, 20.0f);


	c->pitch = 10.0f;
	c->yaw = 0.0f;
	c->position = powf((1 - t), 3) * p0 + 3 * powf((1 - t), 2) * t * p1 + 3 * (1 - t) * powf(t, 2) * p2 + powf(t, 3) * p3;

	c->front.x = cos(glm::radians(c->pitch)) * cos(glm::radians(c->yaw));
	c->front.y = sin(glm::radians(c->pitch));
	c->front.z = cos(glm::radians(c->pitch)) * sin(glm::radians(c->yaw));
}