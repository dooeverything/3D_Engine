#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <SDL.h>
#include <SDL_GL_Window.h>

#include "Picker.h"

class Camera
{
public:
	Camera(glm::vec3 pos, float yaw, float pitch);
	~Camera();

	void processInput();
	void processMouseUp(SDL_Event event, SDL_GL_Window* window);
	void processMouseDown(SDL_Event event);
	void processMouseDrag(SDL_Event event);
	void processPicker(int x, int y);
	void updateCamera();
	void updateProjection();
	void updateSceneProjection();
	void updateView();

	inline glm::mat4 getP() { return m_projectMatrix; };
	inline glm::mat4 getP() const{ return m_projectMatrix; };
	inline glm::mat4 getSP() { return m_scene_projectMatrix; };
	inline glm::mat4 getSP() const { return m_scene_projectMatrix; };
	inline glm::mat4 getV() { return m_viewMatrix; };
	inline glm::mat4 getV() const { return m_viewMatrix; };

	inline glm::vec3 getPos() { return m_pos; };
	inline glm::vec3 getForward() { return m_forward; };
	inline glm::vec3 getUp() { return m_up; };
	inline glm::vec3 getRay() { return m_picker->getRay(); };
	
	inline float getDeltaTime() { return m_delta_time; };
	inline float getLastFrame() { return m_last_frame; };
	inline float getWidth() { return m_width; };
	inline float getHeight() { return m_height; };
	inline float getSceneWidth() { return m_scene_width; };
	inline float getSceneHeight() { return m_scene_height; };

	inline int getMouseX() { return m_mouse_x; };
	inline int getMouseY() { return m_mouse_y; };

	inline void setWidth(float width) { m_width = width; };
	inline void setHeight(float height) { m_height = height; };
	inline void setSceneWidth(float width) { m_scene_width = width; };
	inline void setSceneHeight(float height) { m_scene_height = height; };
	inline void setDeltaTime(float delta_time) { m_delta_time = delta_time; };
	inline void setLastFrame(float last_frame) { m_last_frame = last_frame; };

private:
	unique_ptr<Picker> m_picker;

	glm::mat4 m_projectMatrix;
	glm::mat4 m_scene_projectMatrix;
	glm::mat4 m_viewMatrix;

	glm::vec3 m_pos;
	glm::vec3 m_forward;
	glm::vec3 m_up;

	float m_yaw;
	float m_pitch;
	float m_last_frame;
	float m_delta_time;
	float m_width;
	float m_height;
	float m_scene_width;
	float m_scene_height;

	int m_mouse_x;
	int m_mouse_y;

	bool m_first_click;

};

#endif