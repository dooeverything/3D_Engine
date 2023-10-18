#include "SPHSystemCuda.h"
#include "SPHSolverKernel.cuh"

SPHSystemCuda::SPHSystemCuda(float width, float height, float depth)
{
	m_name = "Fluid";

	cout << endl;
	cout << "*************************Fluid on GPU Information**************************" << endl;
	m_params.grid_cell = 0.10f;
	m_params.H = 0.15f;
	m_params.H2 = m_params.H * m_params.H;
	m_params.POLY6 = 315.0f / float(64.0f * PI * pow(m_params.H, 9));
	m_params.SPICKY = -45.0f / (PI * pow(m_params.H, 6));
	m_params.SPICKY2 = -m_params.SPICKY;
	m_params.MASS = 0.02f;
	m_params.K = 1.0f;
	m_params.rDENSITY = 500.0f;
	m_params.VISC = 2.0f;
	m_params.WALL = -0.5f;
	m_params.SCALE = 1.1f;
	m_params.max_num_neighbors = 5;

	m_simulation = false;
	m_grid_width = width;
	m_grid_height = height;
	m_grid_depth = depth;
	m_fb_width = 1400;
	m_fb_height = 800;

	t = 0.005f;
	render_type = 1;
	iteration = 1;

	initFramebuffer();
	initShader();
	//setupShader();

	m_mesh = make_shared<Mesh>();
	initParticle();

	m_hash = vector<int>(m_particles.size() * 10, -1);
	m_neighbors = vector<int>(m_particles.size() * m_params.max_num_neighbors, -1);
	setHash(m_hash, m_neighbors);

	cout << "Number of particles : " << m_particles.size() << endl;
	cout << "********************************end*********************************" << endl;
	cout << endl;
}

SPHSystemCuda::~SPHSystemCuda()
{
	freeResources();
}

void SPHSystemCuda::initParticle()
{
	srand(1024);
	float particle_seperation = m_params.grid_cell + 0.03f;
	for (float z = 0.0f; z < m_grid_depth; ++z)
	{
		for (float y = 0.0f; y < m_grid_height * 0.5f; ++y)
		{
			for (float x = 0.0f; x < m_grid_width; ++x)
			{
				float ran_x = (float(rand()) / float(RAND_MAX) * 0.5f - 1) * m_params.grid_cell / 10;
				float ran_y = (float(rand()) / float(RAND_MAX) * 0.5f - 1) * m_params.grid_cell / 10;
				float ran_z = (float(rand()) / float(RAND_MAX) * 0.5f - 1) * m_params.grid_cell / 10;

				glm::vec3 pos = glm::vec3(
					x * particle_seperation + ran_x - m_grid_width * m_params.grid_cell / 2.0f,
					y * particle_seperation + ran_y + m_params.grid_cell + 0.5f,
					z * particle_seperation + ran_z - m_grid_depth * m_params.grid_cell / 2.0f
				);

				shared_ptr<FluidParticle> f = make_shared<FluidParticle>(pos);
				f->m_weight = m_params.rDENSITY;
				m_particles.push_back(f);
			}
		}
	}

	float box_x = m_grid_width * m_params.grid_cell;
	float box_y = 2 * m_grid_height * m_params.grid_cell;
	float box_z = m_grid_depth * m_params.grid_cell;
	cout << box_x << " " << box_y << " " << box_z << endl;

	vector<info::VertexLayout> layouts_box;
	for (float x = -box_x; x <= box_x; x += box_x * 2)
	{
		for (float y = 0; y <= box_y; y += box_y)
		{
			for (float z = -box_z; z <= box_z; z += box_z * 2)
			{
				info::VertexLayout layout;
				layout.position = glm::vec3(x, y, z);
				layouts_box.push_back(layout);
			}
		}
	}
	m_mesh->getBuffer().createBuffers(layouts_box);
	m_mesh->computeBoundingBox();


	if (m_point == nullptr)
	{
		vector<info::VertexLayout> layouts;
		for (int i = 0; i < m_particles.size(); ++i)
		{
			info::VertexLayout layout;
			layout.position = m_particles[i]->m_position;
			layouts.emplace_back(layout);
		}
		m_point = make_unique<Point>(layouts);
	}
	else
	{
		vector<info::VertexLayout> layouts = m_point->getMesh().getBuffer().getLayouts();
		for (int i = 0; i < m_particles.size(); ++i)
		{
			info::VertexLayout layout;
			layout.position = m_particles[i]->m_position;
			layouts[i] = layout;
		}
		m_point->getMesh().updateBuffer(layouts);
	}

	computeBlocks(m_particles.size());

	glm::vec3 box = glm::vec3(box_x, box_y, box_z);
	setScale(m_property.at(2));
	setRotation(m_property.at(1));
	setPosition(m_property.at(0));
	glm::vec4 b = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(box, 1.0);

	m_params.box = glm::vec3(b.x, b.y, b.z);
	m_params.t = t;

	setParams(&m_params);

	vector<glm::vec3> pos;
	vector<glm::vec3> vel;
	vector<glm::vec3> force;
	vector<float> density;
	vector<float> pressure;

	for (int i = 0; i < m_particles.size(); ++i)
	{
		pos.push_back(m_particles[i]->m_position);
		vel.push_back(m_particles[i]->m_velocity);
		force.push_back(m_particles[i]->m_force);
		density.push_back(m_particles[i]->m_density);
		pressure.push_back(m_particles[i]->m_pressure);
	}

	copyToCuda(m_particles.size(), pos, vel, force, density, pressure);
}

void SPHSystemCuda::initFramebuffer()
{
	m_fb = make_unique<ShadowBuffer>();
	m_fb->createBuffers(m_fb_width, m_fb_height);

	m_fb_curvature = make_unique<ShadowBuffer>();
	m_fb_curvature->createBuffers(m_fb_width, m_fb_height);

	m_fb_curvature2 = make_unique < ShadowBuffer>();
	m_fb_curvature2->createBuffers(m_fb_width, m_fb_height);

	m_fb_normal = make_unique<FrameBuffer>();
	m_fb_normal->createBuffers(m_fb_width, m_fb_height);
}

void SPHSystemCuda::initShader()
{
	vector<string> depth_shader = { "assets/shaders/Point.vert", "assets/shaders/Point.frag" };
	m_shader_depth = make_unique<Shader>(depth_shader);
	m_shader_depth->processShader();

	vector<string> blur_shader = { "assets/shaders/Debug.vert", "assets/shaders/Smooth.frag" };
	m_screen = make_unique<GameObject>("assets/models/Debug.txt", blur_shader);

	vector<string> curvature_shader = { "assets/shaders/Debug.vert", "assets/shaders/CurvatureFlow.frag" };
	m_shader_curvature = make_unique<Shader>(curvature_shader);
	m_shader_curvature->processShader();

	vector<string> curvature_normal_shader = { "assets/shaders/Debug.vert", "assets/shaders/CurvatureNormal.frag" };
	m_shader_curvature_normal = make_unique<Shader>(curvature_normal_shader);
	m_shader_curvature_normal->processShader();

	vector<string> render_shader = { "assets/shaders/Debug.vert", "assets/shaders/Render.frag" };
	m_shader_render = make_unique<Shader>(render_shader);
	m_shader_render->processShader();
}

void SPHSystemCuda::simulate()
{
	if (!m_simulation) return;

	//cout << "update!" << endl;
	
	float box_x = m_grid_width * m_params.grid_cell;
	float box_z = m_grid_depth * m_params.grid_cell;
	float box_y = 2 * m_grid_height * m_params.grid_cell;
	glm::vec3 box = glm::vec3(box_x, box_y, box_z);
	setScale(m_property.at(2));
	setRotation(m_property.at(1));
	setPosition(m_property.at(0));
	glm::vec4 b = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(box, 1.0);

	m_params.box = glm::vec3(b.x, b.y, b.z);
	m_params.t = t;

	setParams(&m_params);

	vector<glm::vec3> new_pos(m_particles.size());
	simulateCuda(m_particles.size(), t, glm::vec3(b.x, b.y, b.z), new_pos);

	//cout << "** At 10 : " << new_pos[10] << endl;
	//cout << endl;

	// Update positions in a vertex buffer
	vector<info::VertexLayout> layouts = m_point->getMesh().getBuffer().getLayouts();
	for (int i = 0; i < m_particles.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = new_pos[i];
		layouts[i] = layout;
	}
	m_point->getMesh().updateBuffer(layouts);

	m_hash.clear();
	m_neighbors.clear();

	m_hash = vector<int>(info::HASH_SIZE, -1);
	m_neighbors = vector<int>(m_particles.size() * m_params.max_num_neighbors, -1);
	setHash(m_hash, m_neighbors);
}

void SPHSystemCuda::draw()
{
	m_shader_render->load();
	glActiveTexture(GL_TEXTURE0);
	m_fb_normal->bindFrameTexture();
	m_shader_render->setInt("map", 0);
	m_screen->getMesh()->draw();
}

void SPHSystemCuda::setupFramebuffer(const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap, Camera& camera)
{
	glViewport(0, 0, m_fb_width, m_fb_height);
	computeDepth(camera.getSP(), V);
	computeCurvature(camera.getP(), V);
	computeNormal(camera.getP(), V, depth, cubemap);
}

void SPHSystemCuda::computeDepth(const glm::mat4& P, const glm::mat4& V)
{
	float aspect = float(m_fb_width / m_fb_height);
	float point_scale = m_fb_width / aspect * (1.0f / tanf(glm::radians(45.0f)));

	m_fb->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		m_point->getShader().load();
		m_point->getShader().setFloat("point_radius", m_params.grid_cell * m_params.SCALE);
		m_point->getShader().setFloat("point_scale", point_scale);
		m_point->drawPoint(P, V);
	m_fb->unbind();
}

void SPHSystemCuda::computeCurvature(const glm::mat4& P, const glm::mat4& V)
{
	glm::vec2 res = glm::vec2(m_fb_width, m_fb_height);
	m_shader_curvature->load();
	m_shader_curvature->setInt("map", 0);
	m_shader_curvature->setMat4("projection", P);
	m_shader_curvature->setVec2("res", res);

	m_fb_curvature->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_fb->bindFrameTexture();

	m_screen->getMesh()->draw();
	m_fb_curvature->unbind();

	bool swap = false;
	for (int i = 0; i < iteration; ++i)
	{
		if (swap)
		{
			m_fb_curvature->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			m_fb_curvature2->bindFrameTexture();

			m_screen->getMesh()->draw();
			m_fb_curvature->unbind();
		}
		else
		{
			m_fb_curvature2->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			m_fb_curvature->bindFrameTexture();

			m_screen->getMesh()->draw();
			m_fb_curvature2->unbind();
		}

		swap = !swap;
	}
}

void SPHSystemCuda::computeNormal(const glm::mat4& P, const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap)
{
	glm::vec2 inverse_tex = glm::vec2(1.0 / m_fb_width, 1.0 / m_fb_height);

	m_fb_normal->bind();
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_shader_curvature_normal->load();

		m_shader_curvature_normal->setInt("map", 0);
		glActiveTexture(GL_TEXTURE0);
		m_fb_curvature2->bindFrameTexture();

		m_shader_curvature_normal->setInt("depth_map", 1);
		glActiveTexture(GL_TEXTURE1);
		depth.getBuffer().bindFrameTexture();

		m_shader_curvature_normal->setInt("cubemap", 2);
		glActiveTexture(GL_TEXTURE2);
		cubemap.getCubemapBuffer()->bindCubemapTexture();

		m_shader_curvature_normal->setMat4("projection", P);
		m_shader_curvature_normal->setMat4("view", V);
		m_shader_curvature_normal->setVec2("inverse_tex", inverse_tex);
		m_shader_curvature_normal->setInt("render_type", render_type);

		m_screen->getMesh()->draw();
	}
	m_fb_normal->unbind();
}

//glm::ivec3 SPHSystemCuda::getHashPos(const glm::vec3& pos)
//{
//	return { pos.x / m_params.H, pos.y / m_params.H, pos.z / m_params.H };
//}
//
//uint SPHSystemCuda::getHashKey(const glm::ivec3& pos)
//{
//	return ((uint)(pos.x * 73856093) ^
//			(uint)(pos.y * 19349663) ^
//			(uint)(pos.z * 83492791)) % 100000;
//}

//void SPHSystemCuda::fillHash()
//{
	//if (!m_adjs.empty())
	//{
	//	for (int i = 0; i < m_adjs.size(); ++i)
	//	{
	//		m_adjs[i].clear();
	//	}
	//}
	// 
	//m_adjs.clear();
	//m_adjs = vector<<int>>(100000);

	//for (int i = 0; i < m_particles.size(); ++i)
	//{
	//	FluidParticle* p = m_particles[i].get();
	//	glm::ivec3 grid_pos = getHashPos(p->m_position);
	//	uint index = getHashKey(grid_pos);

	//	m_adjs[index].push_back(i);
	//}

	//cout << m_hash.size() << endl;
	//for (int i = 0; i < m_adjs.size(); ++i)
	//{
	//	if (m_adjs[i].empty()) continue;

	//	int n = m_adjs[i].size();
	//	int num_adjs = min(n, 5);
	//	int s = m_adjs[i][0];
	//	//cout << "error " << s << " ";
	//
	//	for (int j = 0; j < num_adjs; ++j)
	//	{
	//		//cout << "at " << s + j * m_particles.size() << " ";
	//		m_hash[s + j*m_particles.size()] = m_adjs[i][j];
	//	}
	//	cout << endl;
	//}
//}
