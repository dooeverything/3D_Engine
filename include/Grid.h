#pragma once

#ifndef GRID_H
#define GRID_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

using namespace std;

class Mesh;

class Grid
{
public:
	Grid();
	void draw(const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos);

private:
	shared_ptr<Mesh> m_mesh;
};

#endif // !GRID_H
