#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_GL_Window.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "Picker.h"

class Camera
{
private:
	unique_ptr<Picker> m_picker;

	glm::mat4 m_viewMatrix;
	glm::vec3 m_pos;
	glm::vec3 m_forward;
	glm::vec3 m_up;

	float m_yaw;
	float m_pitch;

	float m_delta_time;
	float m_last_frame;

	bool m_first_click;
	int m_mouse_x;
	int m_mouse_y;

public:
	Camera(glm::vec3 pos, float yaw, float pitch);
	~Camera();

	void processInput();
	void processMouseUp(SDL_Event event, SDL_GL_Window* window);
	void processMouseDown(SDL_Event event);
	void processMouseDrag(SDL_Event event);
	void processPicker(int w, int h, int x, int y);
	void updateCamera();

	glm::mat4 camera2pixel();

	inline float getDeltaTime() { return m_delta_time; };
	inline float getLastFrame() { return m_last_frame; };
	inline glm::vec3 getPos() { return m_pos; };
	inline glm::vec3 getForward() { return m_forward; };
	inline glm::vec3 getUp() { return m_up; };
	inline int getMouseX() { return m_mouse_x; };
	inline int getMouseY() { return m_mouse_y; };
	inline glm::vec3 getRay() { return m_picker->getRay(); };

	void setDeltaTime(float delta_time) { m_delta_time = delta_time; };
	void setLastFrame(float last_frame) { m_last_frame = last_frame; };
};