#include "BoundingBox.h"
#include "glm/gtx/string_cast.hpp"

bool BoundingBox::intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos)
{
	//cout << glm::to_string(ray_dir) << endl;
	//cout << glm::to_string(ray_pos) << endl;
	//cout << glm::to_string(m_min) << endl;
	//cout << glm::to_string(m_max) << endl;
	
	float temp;
	float tmin = -1.0e10;
	float tmax = 1.0e10;

	// Loop through each axis of the ray
	for (int i = 0; i < 3; ++i)
	{
		float rd = ray_dir[i];

		if (abs(rd) <= 5e-3)
		{
			// if the ray is parallel to the plane, zero division
			continue;
		}

		float v1 = m_min[i] - ray_pos[i];
		float v2 = m_max[i] - ray_pos[i];

		float t1 = v1 / rd;
		float t2 = v2 / rd;

		if (t1 > t2)
		{
			temp = t1;
			t1 = t2;
			t2 = temp;
		}

		if (t1 > tmin)
			tmin = t1;
		if (t2 < tmax)
			tmax = t2;
	}

	if (tmin > tmax) // if the box is missed
	{
		//cout << "Object is missed" << endl;
		return false;
	}

	if (tmax < 0.0f) // if the box is behind the ray
	{
		//cout << "Object is behind the scene" << endl;
		return false;
	}

	t_min = tmin;
	t_max = tmax;

	return true;
}
