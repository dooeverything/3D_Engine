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
	Gizmo(const string& path, const string& name, int type);
	~Gizmo();

	bool clickAxis(glm::vec3 ray_dir, glm::vec3 ray_pos);

	void draw(const glm::mat4& P, const glm::mat4& V, const glm::vec3& view_pos);
	void computeBBox(const glm::vec3& center, const glm::vec3& view_pos);

	void checkDraw(int type);

	inline void setIsAxisRayHit(int c) { m_hit_axis = c; };
	inline void setIsClick(bool click) { m_click = click; };
	inline void setIsDraw(bool draw) { m_is_draw = draw; };

	int getIsAxisRayHit() { return m_hit_axis; };
	bool getIsClick() { return m_click; };
	bool getIsDraw() { return m_is_draw; };

private:
	vector<shared_ptr<Mesh>> m_gizmos;
	vector<glm::mat4> m_Ms;
	glm::vec3 m_center;
	
	string m_name;

	int m_type;
	int m_hit_axis;

	bool m_is_draw;
	bool m_click;
};

#endif // !GIZMO_H