#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace std;

class BoundingBox
{
public:
	BoundingBox() :
		m_min(glm::vec3(0.0f)),
		m_max(glm::vec3(0.0f)),
		t_min(FLT_MIN),
		t_max(FLT_MAX)
	{};

	~BoundingBox() {};

	bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);

	inline void setMin(const glm::vec3& min) { m_min = min; };
	inline void setMax(const glm::vec3& max) { m_max = max; };
	inline void setRayHitMin(float min) { t_min = min; };

	inline glm::vec3 getCenter() const { return (m_min + m_max) / 2.0f; };
	inline glm::vec3 getSize() const { return abs(m_max - m_min); };
	inline glm::vec3 getMin() const { return m_min; };
	inline glm::vec3 getMax() const { return m_max; };
	inline float getRayHitMin() const { return t_min; };

private:
	glm::vec3 m_min;
	glm::vec3 m_max;

	float t_min;
	float t_max;
};
