#include "Gizmo.h"
#include "Object.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "MeshImporter.h"
#include "Mesh.h"

Gizmo::Gizmo() :
	m_center(glm::vec3(0.0f)),
	m_click(false), m_hit_axis(-1)
{
	cout << "Gizmo constructor " << endl;

	shared_ptr<MeshImporter> importer;
	importer = MeshImporter::create("assets/models/ArrowNew.fbx");
	importer->importMesh(m_gizmos);

	const vector<string> shader_paths = { "assets/shaders/Arrow.vert", "assets/shaders/Arrow.frag" };
	ShaderManager::createShader("Arrow", shader_paths);

	m_Ms.resize(4);

	cout << "Mesh complete" << endl;
}

Gizmo::~Gizmo() {}

bool Gizmo::clickAxis(glm::vec3 ray_dir, glm::vec3 ray_pos)
{
	for (int i = 0; i < m_gizmos.size(); ++i)
	{
		if (m_gizmos.at(i)->intersect(ray_dir, ray_pos))
		{
			if (i == 0) break;

			//cout << "click " << i << endl;
			m_hit_axis = i;
			return true;
		}
	}

	m_hit_axis = -1;
	return false;
}

void Gizmo::draw(const glm::mat4& P, const glm::mat4& V, const glm::vec3& view_pos)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Arrow");
	
	glm::vec3 color_x = glm::vec3(225.0f, 10.0f, 99.0f) / 255.0f;	
	if (m_click && m_hit_axis == 1 ) color_x *= 0.5f;
	shader->load();
	shader->setPVM(P, V, m_Ms.at(1));
	shader->setVec3("object_color", color_x);
	m_gizmos.at(1)->draw();

	glm::vec3 color_y = glm::vec3(29.0f, 190.0f, 0.0f) / 255.0f;
	if (m_click && m_hit_axis == 2) color_y *= 0.5f;
	shader->load();
	shader->setPVM(P, V, m_Ms.at(2));
	shader->setVec3("object_color", color_y);
	m_gizmos.at(2)->draw();

	glm::vec3 color_z = glm::vec3(0.0f, 147.0f, 254.0f) / 255.0f;
	if (m_click && m_hit_axis == 3) color_z *= 0.5f;
	shader->load();
	shader->setPVM(P, V, m_Ms.at(3));
	shader->setVec3("object_color", color_z);
	m_gizmos.at(3)->draw();

	glm::vec3 color_cube = glm::vec3(1.0f);
	if (m_click && m_hit_axis == 0) color_cube *= 0.5f;
	shader->load();
	shader->setPVM(P, V, m_Ms.at(0));
	shader->setVec3("object_color", color_cube);
	m_gizmos.at(0)->draw();
}

void Gizmo::computeBBox(const glm::vec3& center, const glm::vec3& view_pos)
{
	m_center = center;

	glm::mat4 M = glm::mat4(1.0f);
	glm::vec3 scale = glm::vec3(glm::length(m_center - view_pos) * 0.05f);
	M = glm::scale(M, scale);
	M = glm::translate(glm::mat4(1.0f), m_center) * M;
	m_Ms.at(1) = M;

	glm::mat4 M_y = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_Ms.at(2) = M_y;

	glm::mat4 M_z = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_Ms.at(3) = M_z;

	scale *= 0.15f;
	M = glm::scale(glm::mat4(1.0f), scale);
	M = glm::translate(glm::mat4(1.0f), m_center) * M;
	m_Ms.at(0) = M;

	for (int i = 0; i < m_gizmos.size(); ++i)
	{
		m_gizmos.at(i)->computeBBox(m_Ms[i]);
	}
}
