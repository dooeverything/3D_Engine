#pragma once

#ifndef POINT_H
#define POINT_H

#include "Mesh.h"

class Point
{
public:
	Point(const vector<info::VertexLayout>& layouts);
	~Point();

	void drawPoint(const glm::mat4& P, const glm::mat4& V);
	inline ParticleMesh& getMesh() { return *m_mesh; };

private:
	unique_ptr<ParticleMesh> m_mesh;
};

#endif // !POINT_H


