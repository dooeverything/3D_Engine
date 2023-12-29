#include "Grid.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "MeshImporter.h"

Grid::Grid()
{
	cout << "Creates Grid" << endl;

	shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Grid.txt");
	importer->importMesh(m_mesh);

	vector<string> shader_paths = { "assets/shaders/Grid.vert", "assets/shaders/Grid.frag" };
	ShaderManager::createShader("Grid", shader_paths);

	cout << "Grid finish loading..." << endl;
	cout << endl;
}

void Grid::draw(const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Grid");

	shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	shader->setPVM(P, V, M);
	shader->setVec3("cam_pos", cam_pos);
	m_mesh->draw();
}
