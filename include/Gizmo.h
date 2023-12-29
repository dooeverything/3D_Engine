#pragma once

#ifndef GIZMO_H
#define GIZMO_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
using namespace std;

class Mesh;

class Gizmo
{
public:
	Gizmo();
	~Gizmo();

	bool clickAxis(glm::vec3 ray_dir, glm::vec3 ray_pos);

	void draw(const glm::mat4& P, const glm::mat4& V, const glm::vec3& view_pos);

	void computeBBox(const glm::vec3& center, const glm::vec3& view_pos);
	void setIsAxisRayHit(int c) { m_hit_axis = c; };
	void setIsClick(bool click) { m_click = click; };

	int getIsAxisRayHit() { return m_hit_axis; };
	bool getIsClick() { return m_click; };

private:
	vector<shared_ptr<Mesh>> m_gizmos;
	vector<glm::mat4> m_Ms;
	glm::vec3 m_center;
	int m_hit_axis;
	bool m_click;
};

#endif // !GIZMO_H