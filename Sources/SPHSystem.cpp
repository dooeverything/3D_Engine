#include "SPHSystem.h"

SPHSystem::SPHSystem(float size) : MarchingCube(size)
{

	cout << "SPHSystem Constructor" << endl;

	m_name = "Fluid Simulation";
	m_grid_size = 1.0f;
	m_threshold = 1.5f;
	m_simulation = false;
	m_render_type = true;

	setParticleRadius(m_grid_size * 2.0f);

	MASS = 1.0f;
	K = 80.0f;
	rDENSITY = 1.0f;
	VISC = 3.0f;
	WALL = -0.5f;

	int num_particles = int(pow(m_size / m_grid_size, 3));
	m_weights = vector<float>(num_particles, 0.0f);
	m_hash_table.reserve(TABLE_SIZE);
	cell_map = vector<bool>(num_particles, false);
	
	m_fb = make_shared<FrameBuffer>();
	m_fb->createBuffers(1400, 800);
	m_fb_blur_x = make_shared<FrameBuffer>();
	m_fb_blur_x->createBuffers(1400, 800);
	m_fb_blur_y = make_shared<FrameBuffer>();
	m_fb_blur_y->createBuffers(1400, 800);

	m_fb_normal = make_shared<FrameBuffer>();
	m_fb_normal->createBuffers(1400, 800);

	initParticles();
	buildHash();

	vector<string> depth_shader = { "Shaders/Point.vert", "Shaders/Point.frag" };
	m_shader = make_shared<Shader>(depth_shader);
	loadShader();

	vector<string> blur_shader = { "Shaders/Debug.vert", "Shaders/Smooth.frag" };
	m_debug = make_unique<GameObject>("Models/Debug.txt", blur_shader);

	vector<string> normal_shader = { "Shaders/Debug.vert", "Shaders/Normal.frag" };
	m_debug2 = make_unique<GameObject>("Models/Debug.txt", normal_shader);

	//vector<string> render_shader = { "Shaders/Debug.vert", "Shaders/Render.frag" };
	//m_debug3 = make_unique<GameObject>("Models/Debug.txt", render_shader);
}

uint SPHSystem::getHashIndex(glm::vec3 pos)
{
	return ((uint)(pos.x * 73856093) ^ 
			(uint)(pos.y * 19349663) ^
			(uint)(pos.z * 83492791)) % TABLE_SIZE;
}

int SPHSystem::getGridIndex(glm::vec3 pos)
{
	int index = int(pos.x + m_size * (pos.y + m_size * pos.z));
	return index;
}

float SPHSystem::getGridValue(int index)
{
	int num_particles = int(pow(m_size / m_grid_size, 3));
	if (index < 0 || index > num_particles)
		return 0.0f;

	float weight = 0.0f;
	try
	{
		weight = m_weights.at(index);
	
	}
	catch(std::out_of_range e)
	{
		cout << "index : " << index << endl;
		cout << "Pos : " << getGridPos(index) << endl;
		assert(0);
	}

	return weight;
}

glm::vec3 SPHSystem::getGridPos(int index)
{
	int z = index / (m_size * m_size);
	index -= z * m_size * m_size;
	int y = index / m_size;
	int x = index % int(m_size);
	return glm::vec3(x, y, z);
}

glm::ivec3 SPHSystem::snapToGrid(glm::vec3 pos)
{
	pos.x = round(pos.x);
	pos.y = round(pos.y);
	pos.z = round(pos.z);
	return pos;
}

void SPHSystem::initParticles()
{
	float fill = 0.5f;

	for (float z = 0.0f; z < m_size; z += m_grid_size)
	{
		for (float y = 0.0f; y < m_size*fill; y += m_grid_size)
		{
			for (float x = 0.0f; x < m_size*fill; x += m_grid_size)
			{
				glm::vec3 pos = { x, y, z };
				m_particles.emplace_back(make_shared<FluidParticle>(pos));
				m_particles.back()->m_density = rDENSITY;
			}
		}
	}
}

void SPHSystem::buildHash()
{
	for (int i = 0; i < TABLE_SIZE; ++i)
	{
		m_hash_table[i] = nullptr;
	}

	for (int i = 0; i < m_particles.size(); ++i)
	{
		if (m_particles[i] == nullptr) continue;
		
		glm::ivec3 grid_pos = snapToGrid(m_particles[i]->m_position);
		uint index = getHashIndex(grid_pos);
		if (m_hash_table.at(index) != nullptr)
		{
			m_particles[i]->m_next = m_hash_table.at(index);
		}
		else
		{
			m_particles[i]->m_next = nullptr;
		}
		m_hash_table.at(index) = m_particles[i];
	}
}

void SPHSystem::createVertex()
{
	for (int i = 0; i < m_weights.size(); ++i)
	{
		if (m_weights[i] >= 1.8) continue;

		glm::vec3 pos = getGridPos(i);
		
		if (pos.x >= m_size || pos.y >= m_size || pos.z >= m_size)
			continue;

		for (float z = -m_grid_size; z <= m_grid_size; z += m_grid_size)
		{
			for (float y = -m_grid_size; y <= m_grid_size; y += m_grid_size)
			{
				for (float x = -m_grid_size; x <= m_grid_size; x += m_grid_size)
				{
					glm::vec3 near_pos = pos + glm::vec3(x, y, z);
					int near_index = getGridIndex(near_pos);
					if (near_pos.x < 0 || near_pos.y < 0 || near_pos.z < 0) continue;
					if (near_pos.x >= m_size || near_pos.y >= m_size || near_pos.z >= m_size) continue;

					if (m_weights.at(near_index) <= 0.0f)
					{
						//cout << pos << endl;
						//cout << "Cell : " << near_index << endl;
						cell_map.at(i) = true;
					}
				
				}
			}
		}
	}

	//cout << "Create edges" << endl;

	for (int i = 0; i < cell_map.size(); ++i)
	{
		if (cell_map[i] == false)
		{
			continue;
		}

		glm::vec3 pos = getGridPos(i);

		if (pos.x >= m_size-2 || pos.y >= m_size-2 || pos.z >= m_size-2)
			continue;

		vector<glm::vec3> gridPoints;
		vector<float> gridValues;

		vector<glm::vec3> points(8);
		vector<float> values(8);

		points[0] = pos;
		values[0] = getGridValue(getGridIndex(points[0]));

		points[1] = { pos.x + m_grid_size, pos.y, pos.z };
		values[1] = getGridValue(getGridIndex(points[1]));

		points[2] = { pos.x + m_grid_size, pos.y, pos.z + m_grid_size };
		values[2] = getGridValue(getGridIndex(points[2]));

		points[3] = { pos.x, pos.y, pos.z + m_grid_size };
		values[3] = getGridValue(getGridIndex(points[3]));

		points[4] = { pos.x, pos.y + m_grid_size, pos.z };
		values[4] = getGridValue(getGridIndex(points[4]));

		points[5] = { pos.x + m_grid_size, pos.y + m_grid_size, pos.z };
		values[5] = getGridValue(getGridIndex(points[5]));

		points[6] = { pos.x + m_grid_size, pos.y + m_grid_size, pos.z + m_grid_size };
		values[6] = getGridValue(getGridIndex(points[6]));

		points[7] = { pos.x, pos.y + m_grid_size, pos.z + m_grid_size };
		values[7] = getGridValue(getGridIndex(points[7]));

		vector<glm::vec3> vertexList(12, { 0, 0, 0 });
		int vertexIndex = 0;

		if (values[0] <= 0.0) vertexIndex |= 1;
		if (values[1] <= 0.0) vertexIndex |= 2;
		if (values[2] <= 0.0) vertexIndex |= 4;
		if (values[3] <= 0.0) vertexIndex |= 8;
		if (values[4] <= 0.0) vertexIndex |= 16;
		if (values[5] <= 0.0) vertexIndex |= 32;
		if (values[6] <= 0.0) vertexIndex |= 64;
		if (values[7] <= 0.0) vertexIndex |= 128;

		if (table::edgeTable[vertexIndex] == 0 || table::edgeTable[vertexIndex] == 255) continue;

		if (table::edgeTable[vertexIndex] & 1)
			vertexList.at(0) = interpolate(points[0], points[1], values[0], values[1], 0.5f);
		if (table::edgeTable[vertexIndex] & 2)
			vertexList.at(1) = interpolate(points[1], points[2], values[1], values[2], 0.5f);
		if (table::edgeTable[vertexIndex] & 4)
			vertexList.at(2) = interpolate(points[2], points[3], values[2], values[3], 0.5f);
		if (table::edgeTable[vertexIndex] & 8)
			vertexList.at(3) = interpolate(points[3], points[0], values[3], values[0], 0.5f);
		if (table::edgeTable[vertexIndex] & 16)
			vertexList.at(4) = interpolate(points[4], points[5], values[4], values[5], 0.5f);
		if (table::edgeTable[vertexIndex] & 32)
			vertexList.at(5) = interpolate(points[5], points[6], values[5], values[6], 0.5f);
		if (table::edgeTable[vertexIndex] & 64)
			vertexList.at(6) = interpolate(points[6], points[7], values[6], values[7], 0.5f);
		if (table::edgeTable[vertexIndex] & 128)
			vertexList.at(7) = interpolate(points[7], points[4], values[7], values[4], 0.5f);
		if (table::edgeTable[vertexIndex] & 256)
			vertexList.at(8) = interpolate(points[0], points[4], values[0], values[4], 0.5f);
		if (table::edgeTable[vertexIndex] & 512)
			vertexList.at(9) = interpolate(points[1], points[5], values[1], values[5], 0.5f);
		if (table::edgeTable[vertexIndex] & 1024)
			vertexList.at(10) = interpolate(points[2], points[6], values[2], values[6], 0.5f);
		if (table::edgeTable[vertexIndex] & 2048)
			vertexList.at(11) = interpolate(points[3], points[7], values[3], values[7], 0.5f);

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

			glm::vec3 n = (n1 + n2 + n3) / 3.0f;

			m_vertices.push_back(a);
			m_vertices.push_back(c);
			m_vertices.push_back(b);

			m_trimeshes.emplace_back(make_shared<TriMesh>(a, b, c));

			m_normals.push_back(n1);
			m_normals.push_back(n1);
			m_normals.push_back(n1);
		}
	}

	if (m_vertices.empty())
	{
		m_mesh = nullptr;
		return;
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < m_vertices.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = m_vertices[i]*0.5f;		
		layout.normal = m_normals[i];
		layouts.push_back(layout);
	}

	m_mesh = make_shared<Mesh>(m_name, layouts);
}

void SPHSystem::update(float t)
{
	if (!m_simulation) return;

	updateDensPress(t);
	updateForces(t);

	for (int i = 0; i < m_particles.size(); ++i)
	{
		if (m_particles[i] == nullptr) continue;
	
		FluidParticle* p = m_particles[i].get();
		glm::vec3 prev_vel = p->m_velocity;
		glm::vec3 prev_pos = p->m_position;
		p->m_velocity += t * (p->m_force/p->m_density  + p->m_gravity) ;
		p->m_position += t * p->m_velocity;		

		p->m_force = glm::vec3(0.0f);
		
		if (p->m_position.x >= m_size)
		{
			p->m_velocity.x *= WALL;
			p->m_position.x = m_size - m_grid_size;
		}
		if (p->m_position.x < 0.0f)
		{
			p->m_velocity.x *= WALL;
			p->m_position.x = 1.0f; //-p->m_position.x + 2 * -1.0f + 0.0001f;
		}

		if (p->m_position.y >= m_size)
		{
			p->m_velocity.y *= WALL;
			p->m_position.y = m_size - m_grid_size;
		}
		if (p->m_position.y <= 1.0f)
		{
			p->m_velocity.y *= WALL;
			p->m_position.y = -p->m_position.y + 2 * 1.0f + 0.0001f; //1.2f;
		}

		if (p->m_position.z >= m_size)
		{
			p->m_velocity.z *= WALL;
			p->m_position.z = m_size - m_grid_size;
		}

		if (p->m_position.z < 0.0f)
		{
			p->m_velocity.z *= WALL;
			p->m_position.z = 1.0f; //-p->m_position.z + 2 * -1.0f + 0.0001f;
		}
	}

	buildHash();
}

void SPHSystem::updateWeights()
{
	for (int i = 0; i < m_weights.size(); ++i)
		m_weights[i] = 0.0f;

	for (int i = 0; i < m_particles.size(); ++i)
	{
		if (m_particles[i] == nullptr) continue;
		FluidParticle* fp = m_particles[i].get();
		glm::vec3 pos = fp->m_position;
		
		// Snap to a grid and update a weight of the grid
		glm::vec3 grid_pos = snapToGrid(pos);
		if (grid_pos.x < 0 || grid_pos.y < 0 || grid_pos.z < 0) continue;
		if (grid_pos.x >= m_size || grid_pos.y >= m_size || grid_pos.z >= m_size) continue;

		int grid_index = getGridIndex(grid_pos);
		m_weights[grid_index] = fp->m_density;

		fp->m_density = rDENSITY;
		fp->m_pressure = 0.0f;
	}
}

void SPHSystem::updateDensPress(float)
{
	for (int i = 0; i < m_particles.size(); ++i)
	{
		float pd = 0.0f;
		glm::vec3 pos = snapToGrid(m_particles[i]->m_position);
		for (float z = -m_grid_size; z <= m_grid_size; z += m_grid_size)
		{
			for (float y = -m_grid_size; y <= m_grid_size; y += m_grid_size)
			{
				for (float x = -m_grid_size; x <= m_grid_size; x += m_grid_size)
				{
					glm::vec3 near_pos = pos + glm::vec3(x, y, z);

					if (near_pos.x < 0 || near_pos.y < 0 || near_pos.z < 0) continue;

					if (near_pos.x >= m_size || near_pos.y >= m_size || near_pos.z >= m_size) continue;

					uint index = getHashIndex(near_pos);
					FluidParticle* near = m_hash_table[index].get();

					while (near != nullptr)
					{

						if (m_particles[i].get() == near)
						{
							near = near->m_next.get();
							break;
						}

						const glm::vec3 diff = pos - near->m_position;
						const float r = glm::length(diff);
						const float r2 = float(pow(r, 2));
						const float h2 = float(pow(H, 2));

						if (r2 <= h2)
						{
							pd += float(MASS * POLY6 * pow(h2 - r2, 3));
						}

						near = near->m_next.get();
					}
				}
			}
		}
		
		m_particles[i]->m_density += pd;
		m_particles[i]->m_density = max(0.0f, m_particles[i]->m_density);
		
		m_particles[i]->m_pressure = K * (m_particles[i]->m_density - rDENSITY);
		m_particles[i]->m_pressure = max(0.0f, m_particles[i]->m_pressure);
	}
}

void SPHSystem::updateForces(float)
{
	for (int i = 0; i < m_particles.size(); ++i)
	{
		glm::vec3 pos = snapToGrid(m_particles[i]->m_position);
		glm::vec3 sum = glm::vec3(0.0f);
		for (float z = -m_grid_size; z <= m_grid_size; z += m_grid_size)
		{
			for (float y = -m_grid_size; y <= m_grid_size; y += m_grid_size)
			{
				for (float x = -m_grid_size; x <= m_grid_size; x += m_grid_size)
				{
					glm::vec3 near_pos = pos + glm::vec3(x, y, z);

					if (near_pos.x < 0 || near_pos.y < 0 || near_pos.z < 0) continue;

					if (near_pos.x >= m_size || near_pos.y >= m_size || near_pos.z >= m_size) continue;

					uint index = getHashIndex(near_pos);
					FluidParticle* near = m_hash_table[index].get();

					while (near != nullptr)
					{
						if (m_particles[i].get() == near)
						{
							near = near->m_next.get();
							continue;
						}

						const glm::vec3 diff = pos - near->m_position;
						const float r = glm::length(diff);
						const float r2 = float(pow(r, 2));
						const float h2 = float(pow(H, 2));

						if (r2 <= h2)
						{
							glm::vec3 dir = normalize(diff);
							glm::vec3 W = float(SPICKY * pow(H - r, 2)) * dir;
							glm::vec3 pressure_force = (MASS / near->m_density) * ((m_particles[i]->m_pressure + near->m_pressure) / 2.0f) * W;

							glm::vec3 v = near->m_velocity - m_particles[i]->m_velocity;
							glm::vec3 viscousity = VISC * (MASS / near->m_density) * v * SPICKY2;

							sum += -pressure_force + viscousity;
						}

						near = near->m_next.get();
					}
				}
			}
		}
	
		m_particles[i]->m_force = sum;
	}
}

void SPHSystem::reset()
{
	m_vertices.clear();
	m_normals.clear();
	m_trimeshes.clear();
	m_particles.clear();
	m_hash_table.clear();
	m_weights.clear();
	cout << "Reset" << endl;
	cout << m_vertices.size() << " " << m_normals.size() << " " << m_trimeshes.size() << " " << m_particles.size() << endl;
	cout << m_hash_table.size() << endl;

	int num_particles = int(pow(m_size / m_grid_size, 3));
	m_weights = vector<float>(num_particles, 0.0f);
	m_hash_table.reserve(TABLE_SIZE);

	initParticles();
	buildHash();
}

void SPHSystem::draw(glm::mat4& P, glm::mat4& V, Light& light, glm::vec3& view_pos, glm::vec3& light_pos, CubeMap& cubemap)
{
	if (m_render_type)
	{
		//vector<glm::mat4> ms;
		//vector<glm::vec3> ps;

		//glm::mat4 m = glm::mat4(1.0f);
		//for (int i = 0; i < m_particles.size(); ++i)
		//{
		//	if (m_particles[i] == nullptr) continue;
		//	glm::vec3 pos = m_particles[i]->m_position;
		//	//cout << pos << endl;
		//	m_particles[i]->m_density = rDENSITY;
		//	m_particles[i]->m_pressure = 0.0f;
		//	m = glm::translate(m, pos*0.5f);
		//	ms.emplace_back(m);
		//	ps.emplace_back(pos);
		//}
		//m_sphere = make_shared<Sphere>(false, ms);
		//
		//m_sphere->drawInstance(P, V, light, view_pos, light_pos);

		//vector<info::VertexLayout> layouts;
		//for (int i = 0; i < ps.size(); ++i)
		//{
		//	info::VertexLayout layout;
		//	layout.position = ps.at(i) * 0.5f;
		//	layouts.emplace_back(layout);
		//}

		//m_point = make_shared<Point>(layouts);

		//m = glm::mat4(1.0f);
		//m_point->drawPoint(P, V, m);
	}
	else
	{		
		updateWeights();

		m_vertices.clear();
		m_normals.clear();
		m_trimeshes.clear();
		createVertex();

		if (m_mesh == nullptr)
			return;

		m_shader->load();
		m_shader->setVec3("view_pos", view_pos);

		// Load shadow map as texture
		m_shader->setInt("skybox", 0);
		glActiveTexture(GL_TEXTURE0);
		cubemap.getCubemapBuffer()->bindCubemapTexture();

		m_mesh->draw(P, V, *m_shader, true);
	}
}

void SPHSystem::setupFrame(glm::mat4& P, glm::mat4& V, float width, float height, float cam_zoom)
{
	vector<glm::vec3> ps;
	vector<glm::mat4> ms;

	for (int i = 0; i < m_particles.size(); ++i)
	{
		glm::mat4 m = glm::mat4(1.0f);
		glm::vec3 pos = m_particles[i]->m_position;
		m = glm::translate(m, pos);
		ms.emplace_back(m);
		ps.emplace_back(pos);

		m_particles[i]->m_density = rDENSITY;
		m_particles[i]->m_pressure = 0.0f;
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < ps.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = ps.at(i) * 0.5f;
		layouts.emplace_back(layout);
	}

	m_sphere = make_shared<Sphere>(false, ms);
	m_point = make_shared<Point>(layouts);

	float aspect = width / height;
	m_fb->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//m_shader->load();
		//setFloat(depth, width / aspectRatio * (1.0f / tanf(cam.zoom * 0.5f)), "pointScale");
		m_point->getShader()->load();
		m_point->getShader()->setFloat("point_radius", 1.0f);
		//m_shader->setFloat("point_radius", 1.8f);
		float point_scale = width / aspect * (1.0f / tanf(glm::radians(45.0f)));
		m_point->getShader()->setFloat("point_scale", point_scale);
		//m_shader->setFloat("point_scale", point_scale);
		//m_sphere->getMesh()->drawInstance(P, V, *m_shader);
		m_point->drawPoint(P, V);
	m_fb->unbind();

	m_fb_blur_x->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_debug->getShader()->load();
		glActiveTexture(GL_TEXTURE0);
		m_fb->bindFrameTexture();
		m_debug->getShader()->setInt("map", 0);
		glm::vec2 dir_x = glm::vec2(1.0 / 100.0, 0.0);
		m_debug->getShader()->setVec2("dir", dir_x);
		m_debug->getMesh()->draw();
	m_fb_blur_x->unbind();

	m_fb_blur_y->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_debug->getShader()->load();
		glActiveTexture(GL_TEXTURE0);
		m_fb_blur_x->bindFrameTexture();
		m_debug->getShader()->setInt("map", 0);
		glm::vec2 dir_y = glm::vec2(0.0, 1.0/100.0);
		m_debug->getShader()->setVec2("dir", dir_y);
		m_debug->getMesh()->draw();
	m_fb_blur_y->unbind();

	//glm::vec3 light_pos = glm::vec3(1.0, 1.0, 1.0);
	m_fb_normal->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_debug2->getShader()->load();
		glActiveTexture(GL_TEXTURE0);
		m_fb_blur_y->bindFrameTexture();
		m_debug2->getShader()->setInt("map", 0);
		m_debug2->getShader()->setMat4("projection", P);
		m_debug2->getShader()->setMat4("view", V);
		glm::vec2 inverse_tex = glm::vec2(1.0 / 100.0, 1.0 / 100.0);
		m_debug2->getShader()->setVec2("inverse_tex", inverse_tex);
		m_debug2->getMesh()->draw();
	m_fb_normal->unbind();
}



