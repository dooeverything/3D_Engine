#include "MarchingCube.h"

// Marching Cube 
// Reference : 
// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu
// https://polycoding.net/marching-cubes/part-2/
// http://paulbourke.net/geometry/polygonise/

MarchingCube::MarchingCube(const string& name) :
	Object(name), m_size(10), m_grid_size(0.1f), m_threshold(1.0f),
	m_vertices({}), m_normals({})
{
}

MarchingCube::MarchingCube(const string& name, float size) :
	Object(name),  m_size(size), m_grid_size(0.1f), m_threshold(1.0f), 
	m_vertices({}), m_normals({})
{
}

MarchingCube::~MarchingCube()
{
}

void MarchingCube::updateVertex()
{
	// Update the vertex
	m_vertices.clear();
	m_normals.clear();
	createVertex();
}

glm::vec3 MarchingCube::interpolate(glm::vec3 grid1, glm::vec3 grid2,
								    float gridValue1, float gridValue2, float threshold)
{
	float newX;
	float newY;
	float newZ;
	float t = (threshold - gridValue1) / (gridValue2 - gridValue1);;

	newX = grid1.x + t * (grid2.x - grid1.x);
	newY = grid1.y + t * (grid2.y - grid1.y);
	newZ = grid1.z + t * (grid2.z - grid1.z);

	return { newX, newY, newZ };
}

void MarchingCube::polygonize(vector<glm::vec3> grids, vector<float> gridValues)
{
	vector<glm::vec3> vertexList(12, { 0, 0, 0 });

	int vertexIndex = 0;
	
	if (gridValues[0] <= m_threshold) vertexIndex |= 1;
	if (gridValues[1] <= m_threshold) vertexIndex |= 2;
	if (gridValues[2] <= m_threshold) vertexIndex |= 4;
	if (gridValues[3] <= m_threshold) vertexIndex |= 8;
	if (gridValues[4] <= m_threshold) vertexIndex |= 16;
	if (gridValues[5] <= m_threshold) vertexIndex |= 32;
	if (gridValues[6] <= m_threshold) vertexIndex |= 64;
	if (gridValues[7] <= m_threshold) vertexIndex |= 128;
	//cout << gridValues[0] << " with " << m_threshold << " : " <<  vertexIndex << endl;
	
	// If the vertex is outside or inside the surface
	if (table::edgeTable[vertexIndex] == 0) return;
	if (table::edgeTable[vertexIndex] & 1)
		vertexList.at(0) = interpolate(grids[0], grids[1], gridValues[0], gridValues[1], m_threshold);
	if (table::edgeTable[vertexIndex] & 2)
		vertexList.at(1) = interpolate(grids[1], grids[2], gridValues[1], gridValues[2], m_threshold);
	if (table::edgeTable[vertexIndex] & 4)
		vertexList.at(2) = interpolate(grids[2], grids[3], gridValues[2], gridValues[3], m_threshold);
	if (table::edgeTable[vertexIndex] & 8)
		vertexList.at(3) = interpolate(grids[3], grids[0], gridValues[3], gridValues[0], m_threshold);
	if (table::edgeTable[vertexIndex] & 16)
		vertexList.at(4) = interpolate(grids[4], grids[5], gridValues[4], gridValues[5], m_threshold);
	if (table::edgeTable[vertexIndex] & 32)
		vertexList.at(5) = interpolate(grids[5], grids[6], gridValues[5], gridValues[6], m_threshold);
	if (table::edgeTable[vertexIndex] & 64)
		vertexList.at(6) = interpolate(grids[6], grids[7], gridValues[6], gridValues[7], m_threshold);
	if (table::edgeTable[vertexIndex] & 128)
		vertexList.at(7) = interpolate(grids[7], grids[4], gridValues[7], gridValues[4], m_threshold);
	if (table::edgeTable[vertexIndex] & 256)
		vertexList.at(8) = interpolate(grids[0], grids[4], gridValues[0], gridValues[4], m_threshold);
	if (table::edgeTable[vertexIndex] & 512)
		vertexList.at(9) = interpolate(grids[1], grids[5], gridValues[1], gridValues[5], m_threshold);
	if (table::edgeTable[vertexIndex] & 1024)
		vertexList.at(10) = interpolate(grids[2], grids[6], gridValues[2], gridValues[6], m_threshold);
	if (table::edgeTable[vertexIndex] & 2048)
		vertexList.at(11) = interpolate(grids[3], grids[7], gridValues[3], gridValues[7], m_threshold);
		
	// Create triangles with vertices on edges
	for (int i = 0; table::triTable[vertexIndex][i] != -1; i += 3)
	{
		glm::vec3 a = vertexList[table::triTable[vertexIndex][i]];
		glm::vec3 b = vertexList[table::triTable[vertexIndex][i + 1]];
		glm::vec3 c = vertexList[table::triTable[vertexIndex][i + 2]];
		
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 n1 = -glm::normalize(glm::cross(ab, ac));
		
		glm::vec3 ba = a - b;
		glm::vec3 bc = c - b;
		glm::vec3 n2 = -glm::normalize(glm::cross(ba, bc));

		glm::vec3 ca = a - c;
		glm::vec3 cb = b - c;
		glm::vec3 n3 = -glm::normalize(glm::cross(ca, cb));

		glm::vec3 n = (n1 + n2 + n3) / glm::length(n1 + n2 + n3);

		m_vertices.push_back(a);
		m_vertices.push_back(c);
		m_vertices.push_back(b);
		
		m_normals.push_back(n1);
		m_normals.push_back(n1);
		m_normals.push_back(n1);
	}
}

Metaball::Metaball(float size) : MarchingCube("Metaball", size)
{
	m_center = glm::vec3(0.f);

	cout << endl;
	cout << "*************************MetaBall Information**************************" << endl;
	cout << "Create Metaball" << endl;
	cout << "Radius : " << m_size << endl;
	//cout << "Center : " << m_center << endl;

	createVertex();

	cout << "********************************end************************************" << endl;
	cout << endl;
}

float Metaball::getGridValue(glm::vec3 p)
{
	/*float x = gridPoint.x - m_center.x;
	float y = gridPoint.y - m_center.y;
	float z = gridPoint.z - m_center.z;*/

	float l = glm::length(m_center - p);
	//cout << "Get grid value From " << m_center << " to " << p << " : " << l << endl;
	return l;
}

void Metaball::createVertex()
{
	vector<glm::vec3> gridPoints;
	vector<float> gridValues;

	for (float z = -m_size; z < m_size; z += m_grid_size)
	{
		for (float y = -m_size; y < m_size; y += m_grid_size)
		{
			for (float x = -m_size; x < m_size; x += m_grid_size)
			{
				gridPoints.push_back({ x, y, z });
				gridValues.push_back(getGridValue(gridPoints.at(0)));

				gridPoints.push_back({ x + m_grid_size, y, z });
				gridValues.push_back(getGridValue({ x + m_grid_size, y, z }));

				gridPoints.push_back({ x + m_grid_size, y, z + m_grid_size });
				gridValues.push_back(getGridValue({ x + m_grid_size, y, z + m_grid_size }));

				gridPoints.push_back({ x, y, z + m_grid_size });
				gridValues.push_back(getGridValue({ x, y, z + m_grid_size }));

				gridPoints.push_back({ x, y + m_grid_size, z });
				gridValues.push_back(getGridValue({ x, y + m_grid_size, z }));

				gridPoints.push_back({ x + m_grid_size, y + m_grid_size, z });
				gridValues.push_back(getGridValue({ x + m_grid_size, y + m_grid_size, z }));

				gridPoints.push_back({ x + m_grid_size, y + m_grid_size, z + m_grid_size });
				gridValues.push_back(getGridValue({ x + m_grid_size, y + m_grid_size, z + m_grid_size }));

				gridPoints.push_back({ x, y + m_grid_size, z + m_grid_size });
				gridValues.push_back(getGridValue({ x, y + m_grid_size, z + m_grid_size }));

				polygonize(gridPoints, gridValues);

				gridPoints.clear();
				gridValues.clear();
			}
		}
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < m_vertices.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = m_vertices[i];
		layout.normal = m_normals[i];
		layouts.push_back(layout);
	}

	shared_ptr<Mesh> mesh = make_shared<Mesh>("Metaball");
	mesh->setupBuffer(layouts);
	addMesh(mesh);
	cout << "Number of Vertices : " << mesh->getSizeVertices() << endl;
}
