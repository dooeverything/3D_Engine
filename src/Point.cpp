#include "Point.h"

Point::Point(const vector<info::VertexLayout>& layouts)
{
	m_mesh = make_unique<ParticleMesh>(layouts);

	vector<string> shader_path = { "assets/shaders/Point.vert", "assets/shaders/Point.frag" };
	m_shader = make_unique<Shader>(shader_path);
	m_shader->processShader();
}

Point::~Point() {}

void Point::drawPoint(const glm::mat4& P, const glm::mat4& V)
{
	m_shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	m_shader->setPVM(P, V, M);

	m_mesh->drawInstance(P, V);
}