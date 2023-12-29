#pragma once

#ifndef QUAD_H
#define QUAD_H

#include "Mesh.h"

class Quad
{
public:
	Quad(Quad const&) = delete;
	Quad& operator=(Quad const&) = delete;

	static Quad* getQuad();

	inline void setMesh(shared_ptr<Mesh> mesh) { m_mesh = mesh; };
	inline void draw() { m_mesh->draw(); }

private:
	shared_ptr<Mesh> m_mesh;
	static unique_ptr<Quad> m_quad;
	Quad();
};

#endif // !QUAD_H
