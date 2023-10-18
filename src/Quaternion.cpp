#include "Quaternion.h"
#include <iostream>

// Reference : 
// https://github.com/gameprogcpp/code/blob/master/Chapter06/Math.h
// https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-29-real-time-rigid-body-simulation-gpus - 29.5 apendix

Quaternion::Quaternion() : m_v(glm::vec4(1.0f)) {}

Quaternion::~Quaternion() {}

void Quaternion::set(glm::vec3 axis, float angle)
{
	float s = glm::sin(angle / 2.0);
	axis *= s;
	m_v.x *= axis.x;
	m_v.y *= axis.y;
	m_v.z *= axis.z;
	m_v.w = glm::cos(angle / 2.0);
}

glm::mat4 Quaternion::getMatrix()
{
	glm::mat4 m;
	m[0][0] = 1.0f - 2.0f * pow(m_v.y, 2) - 2.0f * pow(m_v.z, 2);
	m[0][1] = 2.0f * m_v.x * m_v.y + 2.0f * m_v.w * m_v.z;
	m[0][2] = 2.0f * m_v.x * m_v.z - 2.0f * m_v.w * m_v.y;
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * m_v.x * m_v.y - 2.0f * m_v.w * m_v.z;
	m[1][1] = 1.0f - 2.0f * pow(m_v.x, 2) - 2.0f * pow(m_v.z, 2);
	m[1][2] = 2.0f * m_v.y * m_v.z + 2.0f * m_v.w * m_v.x;
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * m_v.x * m_v.z + 2.0f * m_v.w * m_v.y;
	m[2][1] = 2.0f * m_v.y * m_v.z - 2.0f * m_v.w * m_v.x;
	m[2][2] = 1.0f - 2.0f * pow(m_v.x, 2) - 2.0f * pow(m_v.y, 2);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	
	return m;
}

