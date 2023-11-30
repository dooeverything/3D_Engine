#include "Tri.h"


Tri::Tri(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, 
		 glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3) :
	m_a(v1), m_b(v2), m_c(v3), m_uv1(uv1), m_uv2(uv2), m_uv3(uv3)
{}

glm::vec3 Tri::getClosest(glm::vec3 pos)
{
	float pv1 = glm::length(m_a - pos);
	float pv2 = glm::length(m_b - pos);
	float pv3 = glm::length(m_c - pos);

	float closest = glm::min(pv1, glm::min(pv2, pv3));

	if (closest == pv1)
		return m_a;

	if (closest == pv2)
		return m_b;

	if (closest == pv3)
		return m_c;

	return glm::vec3(-1.0f);
}

bool Tri::intersectWithRay(glm::vec3 ray_dir, glm::vec3 ray_pos, float& t)
{
	glm::vec3 ab = m_b - m_a;
	glm::vec3 ac = m_c - m_a;
	glm::vec3 ap = ray_pos - m_a;
	glm::vec3 n = glm::cross(ab, ac);

	// Check the bad trimesh
	if (n == glm::vec3(0.0f))
		return false;

	n = glm::normalize(n);

	if (ap.y > 0)
		n.y = -n.y;

	float vdotn = glm::dot(n, ray_dir);

	// Intersect from behind
	if (-vdotn < 1.0E-4)
		return false;

	float numer = -glm::dot(ap, n);
	t = numer / vdotn;

	// Intersect from behind
	if (t < 1.0E-4)
		return false;

	float max_norm = FLT_MIN;
	int k = -1;
	for (int i = 0; i < 3; ++i)
	{
		if (glm::abs(n[i]) > max_norm)
		{
			k = i;
			max_norm = n[i];
		}
	}

	glm::vec3 am = ap + t * ray_dir;
	float u = glm::cross(am, ac)[k] / glm::cross(ab, ac)[k];
	float v = glm::cross(ab, am)[k] / glm::cross(ab, ac)[k];
	float a = 1 - u - v;

	if (a < 0 || u < 0 || u > 1 || v < 0 || v > 1)
	{
		return false;
	}

	return true;
}

bool Tri::intersectWithBox(glm::vec3 box_center, glm::vec3 box_r)
{
	//cout << "Intersect with box " << box_center.x << " " << box_center.y << " " << box_center.z << endl;
	//cout << "Intersect with box " << m_b.x << " " << m_b.y << " " << m_b.z << endl;

	glm::vec3 v0 = m_a - box_center;
	glm::vec3 v1 = m_b - box_center;
	glm::vec3 v2 = m_c - box_center;

	glm::vec3 e0 = v1 - v0;
	glm::vec3 e1 = v2 - v1;
	glm::vec3 e2 = v0 - v2;

	float p0, p1, p2, r;

	// Test #1
	p0 = e0.z * v0.y - e0.y * v0.z;
	p2 = e0.z * v2.y - e0.y * v2.z;
	r = box_r.y * abs(e0.z) + box_r.z * abs(e0.y);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = -e0.z * v0.x + e0.x * v0.z;
	p2 = -e0.z * v2.x + e0.x * v2.z;
	r = box_r.x * abs(e0.z) + box_r.z * abs(e0.x);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p1 = e0.y * v0.x - e0.x * v0.y;
	p2 = e0.y * v1.x - e0.x * v1.y;
	r = box_r.x * abs(e0.y) + box_r.y * abs(e0.x);
	if (glm::min(p1, p2) > r || glm::max(p1, p2) < -r) return false;

	// Test #2
	p0 = e1.z * v0.y - e1.y * v0.z;
	p2 = e1.z * v2.y - e1.y * v2.z;
	r = box_r.y * abs(e1.z) + box_r.z * abs(e1.z);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = -e1.z * v0.x + e1.x * v0.z;
	p2 = -e1.z * v2.x + e1.x * v2.z;
	r = box_r.x * abs(e1.z) + box_r.z * abs(e1.x);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = e1.y * v0.x - e1.x * v0.y;
	p1 = e1.y * v1.x - e1.x * v1.y;
	r = box_r.x * abs(e1.y) + box_r.y * abs(e1.x);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	// Test #3
	p0 = e2.x * v0.y - e2.y * v0.z;
	p1 = e2.x * v1.y - e2.y * v1.z;
	r = box_r.y * abs(e2.z) + box_r.z * abs(e2.y);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	p0 = -e2.z * v0.x + e2.x * v0.z;
	p1 = -e2.z * v1.x + e2.x * v1.z;
	r = box_r.x * abs(e2.z) + box_r.z * abs(e2.x);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	p1 = e2.y * v1.x - e2.x * v1.y;
	p2 = e2.y * v2.x - e2.x * v2.y;
	r = box_r.x * abs(e2.y) + box_r.y * abs(e2.x);
	if (glm::min(p1, p2) > r || glm::max(p1, p2) < -r) return false;


	// Test #4
	float min_x = glm::min(v0.x, glm::min(v1.x, v2.x));
	float max_x = glm::max(v0.x, glm::max(v1.x, v2.x));
	if (min_x > box_r.x || max_x < -box_r.x) return false;

	float min_y = glm::min(v0.y, glm::min(v1.y, v2.y));
	float max_y = glm::max(v0.y, glm::max(v1.y, v2.y));
	if (min_y > box_r.y || max_y < -box_r.y) return false;

	float min_z = glm::min(v0.z, glm::min(v1.z, v2.z));
	float max_z = glm::max(v0.z, glm::max(v1.z, v2.z));
	if (min_z > box_r.z || max_z < -box_r.z) return false;

	// Test 5
	glm::vec3 n = glm::cross(e0, e1);
	glm::vec3 tmin = glm::vec3(0.0f);
	glm::vec3 tmax = glm::vec3(0.0f);

	for (int i = 0; i < 3; ++i)
	{
		float dir = n[i];

		if (n[i] > 0.0f)
		{
			tmin[i] = -box_r[i] - dir;
			tmax[i] = box_r[i] - dir;
		}
		else
		{
			tmin[i] = box_r[i] - dir;
			tmax[i] = -box_r[i] - dir;
		}
	}

	if (glm::dot(n, tmin) > 0.0f) return false; // not overlap
	else if (glm::dot(n, tmax) >= 0.0f) return true; // overlap
	else return false;

	return true;
}