#include "Geometry.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Light.h"

Geometry::Geometry(const string& name) : Object(name)
{}

Geometry::~Geometry()
{}

Sphere::Sphere(bool is_create_gizmo) :
	Geometry("Sphere"), m_division(32.0f), m_radius(1.0f)
{
	cout << "********************Create Sphere Constructor********************" << endl;

	vector<info::VertexLayout> layouts = calculateVertex();
	vector<unsigned int> indices = calculateIndex();
	shared_ptr<Mesh> mesh = make_shared<Mesh>("Sphere");
	mesh->setupBuffer(layouts, indices);
	addMesh(mesh);

	cout << "********************Finish create sphere********************\n" << endl;
}

Sphere::~Sphere()
{
	cout << "Delete Sphere" << endl;
}


void Sphere::drawPreview(const Material& mat)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Default");

	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	shader->load();
	shader->setVec3("view_pos", view_pos);
	shader->setLight(*light);
	shader->setMaterial(mat);
	shader->setInt("type", 3);

	shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	drawMesh(P, V, *shader);

	glActiveTexture(GL_TEXTURE0);
}

void Sphere::drawPreview(const vector<shared_ptr<Texture>>& tex)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Preview");

	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	shader->load();
	shader->setVec3("view_pos", view_pos);
	shader->setLight(*light);
	shader->setPVM(P, V, M);

	shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	//shader->setInt("type", 1);
	//shader->setInt("texture_map", 4);
	//glActiveTexture(GL_TEXTURE0 + 4);
	//tex[0]->setActive();

	drawMesh(P, V, *shader);

	glActiveTexture(GL_TEXTURE0);
}


vector<info::VertexLayout> Sphere::calculateVertex()
{
	float angle_sector = float(2 * M_PI) / m_division; // 0 to 360
	float angle_stack = float(M_PI / m_division); // 90 to -90

	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> texCoords;

	for (int i = 0; i <= m_division; i++) // stack
	{
		float u = (float)(M_PI / 2) - (float)angle_stack * (float)i;
		for (int j = 0; j <= m_division; j++) // Sector
		{
			float v = (float)j * angle_sector;

			float x = cos(u) * sin(v);
			float y = sin(u);
			float z = cos(u) * cos(v);

			glm::vec3 vertex = { x, y, z };
			vertices.push_back(vertex);

			glm::vec3 normal = { x, y, z };
			normals.push_back(normal);

			float lon = glm::atan(z, x);
			float lat = glm::atan(y, sqrt(x * x + z * z));
			float s = float(lon + M_PI) / float(2 * M_PI);
			float t = float(log(tan(lat / 2 + M_PI / 4)) + M_PI) / float(2 * M_PI);

			glm::vec2 texCoord = { s, t };
			texCoords.push_back(texCoord);
		}
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < vertices.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = vertices[i] * m_radius;
		layout.normal = normals[i];
		layout.texCoord = texCoords[i];
		layouts.push_back(layout);
	}

	return layouts;
}

vector<unsigned int> Sphere::calculateIndex()
{
	vector<unsigned int> indices;
	unsigned int I1;
	unsigned int I2;

	for (int i = 0; i < m_division; ++i) { // Stack
		I1 = i * unsigned int(m_division + 1);
		I2 = I1 + unsigned int(m_division + 1);
		for (int j = 0; j < m_division; ++j) { // Sector
			if (i != 0)
			{
				indices.push_back(I1);
				indices.push_back(I2);
				indices.push_back(I1 + 1);
			}
			if (i != m_division - 1)
			{
				indices.push_back(I1 + 1);
				indices.push_back(I2);
				indices.push_back(I2 + 1);
			}
			I1 += 1;
			I2 += 1;
		}
	}

	return indices;
}
