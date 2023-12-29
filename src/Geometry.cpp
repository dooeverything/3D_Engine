#include "Geometry.h"
#include "Mesh.h"
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
