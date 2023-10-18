#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Quaternion
{
public:
	Quaternion();
	~Quaternion();

	void set(glm::vec3, float angle);
	glm::mat4 getMatrix();
	//inline float getAngle() { return m_angle; };

private:
	//float m_angle;
	glm::vec4 m_v;
};