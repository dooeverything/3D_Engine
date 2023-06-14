#pragma once

#ifndef PICKER_H
#define PICKER_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

// Inspired and modified from 
// 1. https://www.dropbox.com/s/qkslys3p3xzh8av/MousePicker%20Code.txt?dl=0, 
// 2. https://antongerdelan.net/opengl/raycasting.html

class Picker
{
private:
	glm::vec3 m_ray_world;

public:
	Picker();
	~Picker();

	void calcualteMouseRay(int width, int height, 
						   int mouse_x, int mouse_y,
						   glm::mat4 view);
	glm::vec2 getNDS(int width, int height, int mouse_x, int mouse_y);
	glm::vec4 getCameraSpace(glm::vec4 clip_coord, float aspect);
	glm::vec3 getWorldSpace(glm::vec4 camera_coord, glm::mat4 view);

	inline glm::vec3 getRay() { return m_ray_world; };
};

#endif
