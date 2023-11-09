#include "MarchingCube.h"
#include "Utils.h"

// Marching Cube 
// Reference : 
// https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-1-generating-complex-procedural-terrains-using-gpu
// https://polycoding.net/marching-cubes/part-2/
// http://paulbourke.net/geometry/polygonise/

Tri::Tri(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) :
	m_a(v1), m_b(v2), m_c(v3)
{}

glm::vec3 Tri::getClosest(glm::vec3 pos)
{
	float pv1 = glm::length(m_a - pos);
	float pv2 = glm::length(m_b - pos);
	float pv3 = glm::length(m_c - pos);

	float closest = glm::min(pv1, glm::min(pv2, pv3));

	if (closest == pv1)
		return m_a;

	if (closest == pv2)
		return m_b;

	if (closest == pv3)
		return m_c;

	return glm::vec3(-1.0f);
}

bool Tri::intersectWithRay(glm::vec3 ray_dir, glm::vec3 ray_pos, float& t)
{
	glm::vec3 ab = m_b - m_a;
	glm::vec3 ac = m_c - m_a;
	glm::vec3 ap = ray_pos - m_a;
	glm::vec3 n = glm::cross(ab, ac);

	// Check the bad trimesh
	if (n == glm::vec3(0.0f))
		return false;

	n = glm::normalize(n);

	if (ap.y > 0)
		n.y = -n.y;

	// Intersect from behind
	float vdotn = glm::dot(n, ray_dir);
	if (-vdotn < 0.00001f)
	{
		return false;
	}

	float numer = -glm::dot(ap, n);
	t = numer / vdotn;

	// Intersect from behind
	if (t < 0.00001f)
		return false;

	float max_norm = FLT_MIN;
	int k = -1;
	for (int i = 0; i < 3; ++i)
	{
		if (glm::abs(n[i]) > max_norm)
		{
			k = i;
			max_norm = n[i];
		}
	}

	glm::vec3 am = ap + t * ray_dir;
	float u = glm::cross(am, ac)[k] / glm::cross(ab, ac)[k];
	float v = glm::cross(ab, am)[k] / glm::cross(ab, ac)[k];
	float a = 1 - u - v;

	if (a < 0 || u < 0 || u > 1 || v < 0 || v > 1)
	{
		return false;
	}

	return true;
}

bool Tri::intersectWithBox(glm::vec3 box_center, glm::vec3 box_r)
{
	//cout << "Intersect with box " << box_center.x << " " << box_center.y << " " << box_center.z << endl;
	//cout << "Intersect with box " << m_b.x << " " << m_b.y << " " << m_b.z << endl;

	glm::vec3 v0 = m_a - box_center;
	glm::vec3 v1 = m_b - box_center;
	glm::vec3 v2 = m_c - box_center;

	glm::vec3 e0 = v1 - v0;
	glm::vec3 e1 = v2 - v1;
	glm::vec3 e2 = v0 - v2;

	float p0, p1, p2, r;

	// Test #1
	p0 = e0.z * v0.y - e0.y * v0.z;
	p2 = e0.z * v2.y - e0.y * v2.z;
	r = box_r.y * abs(e0.z) + box_r.z * abs(e0.y);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = -e0.z * v0.x + e0.x * v0.z;
	p2 = -e0.z * v2.x + e0.x * v2.z;
	r = box_r.x * abs(e0.z) + box_r.z * abs(e0.x);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p1 = e0.y * v0.x - e0.x * v0.y;
	p2 = e0.y * v1.x - e0.x * v1.y;
	r = box_r.x * abs(e0.y) + box_r.y * abs(e0.x);
	if (glm::min(p1, p2) > r || glm::max(p1, p2) < -r) return false;

	// Test #2
	p0 = e1.z * v0.y - e1.y * v0.z;
	p2 = e1.z * v2.y - e1.y * v2.z;
	r = box_r.y * abs(e1.z) + box_r.z * abs(e1.z);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = -e1.z * v0.x + e1.x * v0.z;
	p2 = -e1.z * v2.x + e1.x * v2.z;
	r = box_r.x * abs(e1.z) + box_r.z * abs(e1.x);
	if (glm::min(p0, p2) > r || glm::max(p0, p2) < -r) return false;

	p0 = e1.y * v0.x - e1.x * v0.y;
	p1 = e1.y * v1.x - e1.x * v1.y;
	r = box_r.x * abs(e1.y) + box_r.y * abs(e1.x);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	// Test #3
	p0 = e2.x * v0.y - e2.y * v0.z;
	p1 = e2.x * v1.y - e2.y * v1.z;
	r = box_r.y * abs(e2.z) + box_r.z * abs(e2.y);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	p0 = -e2.z * v0.x + e2.x * v0.z;
	p1 = -e2.z * v1.x + e2.x * v1.z;
	r = box_r.x * abs(e2.z) + box_r.z * abs(e2.x);
	if (glm::min(p0, p1) > r || glm::max(p0, p1) < -r) return false;

	p1 = e2.y * v1.x - e2.x * v1.y;
	p2 = e2.y * v2.x - e2.x * v2.y;
	r = box_r.x * abs(e2.y) + box_r.y * abs(e2.x);
	if (glm::min(p1, p2) > r || glm::max(p1, p2) < -r) return false;


	// Test #4
	float min_x = glm::min(v0.x, glm::min(v1.x, v2.x));
	float max_x = glm::max(v0.x, glm::max(v1.x, v2.x));
	if (min_x > box_r.x || max_x < -box_r.x) return false;

	float min_y = glm::min(v0.y, glm::min(v1.y, v2.y));
	float max_y = glm::max(v0.y, glm::max(v1.y, v2.y));
	if (min_y > box_r.y || max_y < -box_r.y) return false;

	float min_z = glm::min(v0.z, glm::min(v1.z, v2.z));
	float max_z = glm::max(v0.z, glm::max(v1.z, v2.z));
	if (min_z > box_r.z || max_z < -box_r.z) return false;

	// Test 5
	glm::vec3 n = glm::cross(e0, e1);
	glm::vec3 tmin = glm::vec3(0.0f);
	glm::vec3 tmax = glm::vec3(0.0f);

	for (int i = 0; i < 3; ++i)
	{
		float dir = n[i];

		if (n[i] > 0.0f)
		{
			tmin[i] = -box_r[i] - dir;
			tmax[i] = box_r[i] - dir;
		}
		else
		{
			tmin[i] = box_r[i] - dir;
			tmax[i] = -box_r[i] - dir;
		}
	}

	if (glm::dot(n, tmin) > 0.0f) return false; // not overlap
	else if (glm::dot(n, tmax) >= 0.0f) return true; // overlap
	else return false;

	return true;
}


MarchingCube::MarchingCube() :
	GameObject(), m_size(10), m_grid_size(0.1f), m_threshold(1.0f),
	m_vertices({}), m_normals({}), m_trimeshes({})
{
}

MarchingCube::MarchingCube(float size) :
	GameObject(),  m_size(size), m_grid_size(0.1f), m_threshold(1.0f), 
	m_vertices({}), m_normals({}), m_trimeshes({})
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
	m_trimeshes.clear();
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

		m_trimeshes.emplace_back(make_shared<Tri>(a, b, c));

		m_normals.push_back(n1);
		m_normals.push_back(n1);
		m_normals.push_back(n1);
	}
}

Metaball::Metaball(float size) : MarchingCube(size)
{
	m_name = "Metaball";
	m_center = glm::vec3(0.f);

	cout << endl;
	cout << "*************************MetaBall Information**************************" << endl;
	cout << "Create Metaball" << endl;
	cout << "Radius : " << m_size << endl;
	cout << "Center : " << m_center << endl;

	createVertex();

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

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

	m_mesh = make_shared<Mesh>(m_name);
	m_mesh->getBuffer().createBuffers(layouts);
	m_mesh->computeBoundingBox();

	cout << "Number of Vertices : " << m_mesh->getBuffer().getLayouts().size() << endl;
}

Terrain::Terrain(float size) : MarchingCube(size)
{
	cout << "Terrain Constructor" << endl;

	m_name = "Terrain";
	m_grid_size = 1.0f;
	m_threshold = 0.5f;

	m_noise_scale = 1;
	m_octaves = 8;
	m_frequency = 0.02f;
	m_is_edit = false;
	m_brush_size = 1.0f;
	m_strength = 1.0f;

	createWeights();
	createVertex();

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	cout << "Terrain Constructor successfullly loaded" << endl;
	cout << endl;
}

float Terrain::getGridValue(glm::vec3 grid_point)
{
	int index = int(grid_point.x + m_size * (grid_point.y + m_size * grid_point.z));
	return m_weights[index];
}

void Terrain::createVertex()
{
	for (float z = 0.0f; z < m_size-1; z += m_grid_size)
	{
		for (float y = 0.0f; y < m_size-1; y += m_grid_size)
		{
			for (float x = 0.0f; x < m_size - 1; x += m_grid_size)
			{
				vector<glm::vec3> points(8);
				vector<float> values(8);

				points[0] = { x, y, z };
				values[0] = getGridValue(points[0]);

				points[1] = { x + m_grid_size, y, z };
				values[1] = getGridValue(points[1]);

				points[2] = { x + m_grid_size, y, z + m_grid_size };
				values[2] = getGridValue(points[2]);

				points[3] = { x, y, z + m_grid_size };
				values[3] = getGridValue(points[3]);

				points[4] = { x, y + m_grid_size, z };
				values[4] = getGridValue(points[4]);

				points[5] = { x + m_grid_size, y + m_grid_size, z };
				values[5] = getGridValue(points[5]);

				points[6] = { x + m_grid_size, y + m_grid_size, z + m_grid_size };
				values[6] = getGridValue(points[6]);

				points[7] = { x, y + m_grid_size, z + m_grid_size };
				values[7] = getGridValue(points[7]);

				polygonize(points, values);
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

	m_mesh = make_shared<Mesh>(m_name);
	m_mesh->getBuffer().createBuffers(layouts);
	m_mesh->computeBoundingBox();
}

void Terrain::createWeights()
{
	m_weights.clear();
	int num_weights = int(pow(m_size, 3.0));
	m_weights = vector<float>(num_weights);
	cout << "Num weights : " << num_weights << endl;

	FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
	noise.SetFrequency(m_frequency);
	noise.SetFractalOctaves(m_octaves);

	for (float z = 0.0f; z < m_size; z += m_grid_size)
	{
		for (float y = 0.0f; y < m_size; y += m_grid_size)
		{
			for (float x = 0.0f; x < m_size; x += m_grid_size)
			{
				glm::vec3 pos = { x, y, z };
				pos *= m_noise_scale;

				float ground = -pos.y + (0.5f * m_size);
				float n = ground + noise.GetNoise(pos.x, pos.y, pos.z) * 10;
				int index = int(x + m_size * (y + m_size * z));
				m_weights[index] = n;
			}
		}
	}
}

void Terrain::updateWeights(glm::vec3 ray_dir, glm::vec3 ray_pos)
{
	float t = 0.0f;
	
	std::sort(m_trimeshes.begin(), m_trimeshes.end(),
		[ray_pos](const shared_ptr<Tri>& lhs, const shared_ptr<Tri>& rhs)
		{
			float d1 = glm::length(ray_pos - (lhs->getClosest(ray_pos)));
			float d2 = glm::length(ray_pos - (rhs->getClosest(ray_pos)));
			return d1 < d2;
		});

	glm::vec3 hit_pos = glm::vec3(0.0f);
	for (int i = 0; i < m_trimeshes.size(); ++i)
	{
		if (m_trimeshes[i]->intersectWithRay(ray_dir, ray_pos, t))
		{
			hit_pos = ray_pos + ray_dir * t;
			int index = int(hit_pos.x + m_size * (hit_pos.y + m_size * hit_pos.z));
			break;
		}
	}

	for (float z = 0.0f; z < m_size; z += m_grid_size)
	{
		for (float y = 0.0f; y < m_size; y += m_grid_size)
		{
			for (float x = 0.0f; x < m_size; x += m_grid_size)
			{
				glm::vec3 p = { x, y, z };
				float d = glm::length(hit_pos - p);
				if (d <= m_brush_size)
				{
					int index = int(x + m_size * (y + m_size * z));
					m_weights[index] += m_strength;
				}
			}
		}
	}

	updateVertex();
}

void Terrain::renderProperty()
{
	bool expand_terrain = ImGui::TreeNode("Terrain");
	if (expand_terrain)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		ImVec2 cell_padding(0.0f, 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
		ImGui::BeginTable("Terrain", 2);
		ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

		bool update = false;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Size");
		ImGui::TableNextColumn();
		string id_size = "##size";
		if (ImGui::SliderFloat(id_size.c_str(), &m_size, 0.0f, 1.0f, "%.1f", 0))
		{
			//t->setSize(size);
			update = true;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Grid Size");
		ImGui::TableNextColumn();
		string id_grid = "##grid";
		if (ImGui::SliderFloat(id_grid.c_str(), &m_grid_size, 0.0f, 1.0f, "%.1f", 0))
		{
			//t->setGridSize(grid_size);
			update = true;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Threshold");
		ImGui::TableNextColumn();
		string id_threshold = "##threshold";
		if (ImGui::SliderFloat(id_threshold.c_str(), &m_threshold, 0.0f, 1.0f, "%.1f", 0))
		{
			//t->setThreshold(threshold);
			update = true;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Noise Scale");
		ImGui::TableNextColumn();
		string id_scale = "##noiseScale";
		if (ImGui::SliderInt(id_scale.c_str(), &m_noise_scale, 1, 10))
		{
			//t->setNoiseScale(mnoise_scale);
			update = true;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Octaves");
		ImGui::TableNextColumn();
		string id_octave = "##octaves";
		if (ImGui::SliderInt(id_octave.c_str(), &m_octaves, 1, 20))
		{
			//t->setOctave(octaves);
			update = true;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Frequency");
		ImGui::TableNextColumn();
		string id_freq = "##frequency";
		if (ImGui::SliderFloat(id_freq.c_str(), &m_threshold, 0.0f, 1.0f, "%.3f", 0))
		{
			//t->setFrequency(frequency);
			update = true;
		}

		if (update)
		{
			createWeights();
			updateVertex();
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		static int clicked = 0;
		if (ImGui::Button("Click to edit terrain"))
		{
			clicked++;
		}

		static int selected = -1;
		if (clicked & 1)
		{
			//t->setIsEdit(true);
			m_is_edit = true;
			ImGui::TableNextColumn();
			char buf1[32];
			sprintf_s(buf1, "Sculpting");
			if (ImGui::Selectable(buf1, selected == 0, 0, ImVec2(64, 16)))
				selected = 0;

			//ImGui::TableNextColumn();
			char buf2[32];
			sprintf_s(buf2, "Remove");
			if (ImGui::Selectable(buf2, selected == 1, 0, ImVec2(64, 16)))
				selected = 1;

		}
		else
		{
			selected = -1;
			//t->setIsEdit(false);
			m_is_edit = false;
		}

		if (selected == 0)
		{
			//t->setStrength(1.0f);
			m_strength = 1.0f;
		}
		else if (selected == 1)
		{
			//t->setStrength(-1.0f);
			m_strength = -1.0f;
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
		ImGui::TreePop();
	}
	else
	{
		//t->setIsEdit(false);
		m_is_edit = false;
	}
}
