#pragma once

#ifndef QUATERNION_H
#define QUATERNION_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Quaternion
{
public:
	Quaternion();
	~Quaternion();

	void set(glm::vec3, float angle);
	glm::mat4 getMatrix();

private:
	glm::vec4 m_v;
};

#endif // !QUATERNION_H