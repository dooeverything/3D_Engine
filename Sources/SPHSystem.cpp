#include "SPHSystem.h"

SPHSystem::SPHSystem(float size, int width, int height) : MarchingCube(size)
{
	cout << "SPHSystem Constructor" << endl;

	m_name = "Fluid Simulation";
	m_grid_size = 1.0f;
	m_threshold = 1.5f;
	m_simulation = false;
	m_render_type = true;
	m_width = width;
	m_height = height;

	setParticleRadius(2.0f);
	MASS = 1.0f;
	K = 80.0f;
	rDENSITY = 1.0f;
	VISC = 3.0f;
	WALL = -0.5f;


	int num_particles = int(pow(m_size / m_grid_size, 3));
	//m_weights = vector<float>(num_particles, 0.0f);
	m_hash_table.reserve(TABLE_SIZE);
	cell_map = vector<bool>(num_particles, false);

	m_fb = make_shared<ShadowBuffer>();
	m_fb->createBuffers(width, height);

	m_fb_blur_x = make_shared<ShadowBuffer>();
	m_fb_blur_x->createBuffers(width, height);
	m_fb_blur_y = make_shared<ShadowBuffer>();
	m_fb_blur_y->createBuffers(width, height);

	m_fb_curvature = make_shared<ShadowBuffer>();
	m_fb_curvature->createBuffers(width, height);

	m_fb_normal = make_shared<FrameBuffer>();
	m_fb_normal->createBuffers(width, height);

	initParticles();
	buildHash();

	vector<string> depth_shader = { "Shaders/Point.vert", "Shaders/Point.frag" };
	m_shader = make_shared<Shader>(depth_shader);
	loadShader();

	vector<string> blur_shader = { "Shaders/Debug.vert", "Shaders/Smooth.frag" };
	m_screen = make_unique<GameObject>("Models/Debug.txt", blur_shader);

	vector<string> curvature_shader = { "Shaders/Debug.vert", "Shaders/CurvatureFlow.frag" };
	m_shader_curvature = make_shared<Shader>(curvature_shader);
	m_shader_curvature->processShader();

	vector<string> curvature_normal_shader = { "Shaders/Debug.vert", "Shaders/CurvatureNormal.frag" };
	m_shader_curvature_normal = make_shared<Shader>(curvature_normal_shader);
	m_shader_curvature_normal->processShader();

	vector<string> normal_shader = { "Shaders/Debug.vert", "Shaders/Normal.frag" };
	m_shader_normal = make_shared<Shader>(normal_shader);
	m_shader_normal->processShader();
	
	vector<string> render_shader = { "Shaders/Debug.vert", "Shaders/Render.frag" };
	m_shader_render = make_shared<Shader>(render_shader);
	m_shader_render->processShader();
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
		m_shader_render->load();
		m_fb_normal->bindFrameTexture();
		m_shader_render->setInt("map", 0);
		m_screen->getMesh()->draw();
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

void SPHSystem::setupFrame(glm::mat4& P, glm::mat4& V, CubeMap& cubemap, int width, int height)
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
		layout.position = ps.at(i) * 0.05f;
		layouts.emplace_back(layout);
	}

	//m_sphere = make_shared<Sphere>(false, ms);
	m_point = make_shared<Point>(layouts);

	getDepth(P, V);
	blurDepth();
	//getNormal(P, V, cubemap);

	m_fb_curvature->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shader_curvature->load();

		m_shader_curvature->setInt("map", 0);
		glActiveTexture(GL_TEXTURE0);
		m_fb_blur_y->bindFrameTexture();

		m_shader_curvature->setMat4("projection", P);

		glm::vec2 screen_size = glm::vec2(m_width, m_height);
		m_shader_curvature->setVec2("screen_size", screen_size);
		m_shader_curvature->setFloat("dt", 0.005f);

		m_screen->getMesh()->draw();
	m_fb_curvature->unbind();


	m_fb_normal->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shader_curvature_normal->load();

		m_shader_curvature_normal->setInt("map", 0);
		glActiveTexture(GL_TEXTURE0);
		m_fb_curvature->bindFrameTexture();

		m_shader_curvature_normal->setInt("cubemap", 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		cubemap.getCubemapBuffer()->bindCubemapTexture();

		m_shader_curvature_normal->setMat4("projection", P);
		m_shader_curvature_normal->setMat4("view", V);
		glm::vec2 inverse_tex = glm::vec2(1.0 / m_width, 1.0 / m_height);
		m_shader_curvature_normal->setVec2("inverse_tex", inverse_tex);
		
		m_screen->getMesh()->draw();
	m_fb_normal->unbind();


}

void SPHSystem::createVertex()
{
	return;
}

void SPHSystem::getDepth(glm::mat4& P, glm::mat4& V)
{
	float aspect = m_width / m_height;
	m_fb->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		m_point->getShader()->load();

		m_point->getShader()->setFloat("point_radius", 0.05f);
		float point_scale = m_width / aspect * (1.0f / tanf(glm::radians(45.0f)));
		m_point->getShader()->setFloat("point_scale", point_scale);

		m_point->drawPoint(P, V);
	m_fb->unbind();
}

void SPHSystem::blurDepth()
{
	m_fb_blur_x->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_screen->getShader()->load();

		glActiveTexture(GL_TEXTURE0);
		m_fb->bindFrameTexture();
		m_screen->getShader()->setInt("map", 0);

		glm::vec2 dir_x = glm::vec2(1.0 / m_width, 0.0);
		m_screen->getShader()->setVec2("dir", dir_x);

		m_screen->getMesh()->draw();
	m_fb_blur_x->unbind();

	m_fb_blur_y->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_screen->getShader()->load();

		glActiveTexture(GL_TEXTURE0);
		m_fb_blur_x->bindFrameTexture();
		m_screen->getShader()->setInt("map", 0);

		glm::vec2 dir_y = glm::vec2(0.0, 1.0 / m_height);
		m_screen->getShader()->setVec2("dir", dir_y);

		m_screen->getMesh()->draw();
	m_fb_blur_y->unbind();
}

void SPHSystem::getNormal(glm::mat4& P, glm::mat4& V, CubeMap& cubemap)
{
	m_fb_normal->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_shader_normal->load();

		m_shader_normal->setInt("map", 0);
		glActiveTexture(GL_TEXTURE0);
		m_fb_blur_y->bindFrameTexture();

		m_shader_normal->setInt("cubemap", 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		cubemap.getCubemapBuffer()->bindCubemapTexture();

		m_shader_normal->setMat4("projection", P);
		m_shader_normal->setMat4("view", V);
		glm::vec2 inverse_tex = glm::vec2(1.0 / m_width, 1.0 / m_height);
		m_shader_normal->setVec2("inverse_tex", inverse_tex);
		m_screen->getMesh()->draw();
	m_fb_normal->unbind();
}