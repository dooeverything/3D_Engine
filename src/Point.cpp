#include "Point.h"
#include "ShaderManager.h"
#include "Shader.h"

Point::Point(const vector<info::VertexLayout>& layouts)
{
	m_mesh = make_unique<ParticleMesh>(layouts);

	vector<string> shader_paths = { "assets/shaders/Point.vert", "assets/shaders/Point.frag" };
	ShaderManager::createShader("Point", shader_paths);
}

Point::~Point() {}

void Point::drawPoint(const glm::mat4& P, const glm::mat4& V)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Point");

	shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	shader->setPVM(P, V, M);

	m_mesh->drawInstance();
}