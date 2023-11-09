#include "SPHSystemCuda.h"
#include "SPHSolverKernel.cuh"

SPHSystemCuda::SPHSystemCuda(float width, float height, float depth)
{
	m_name = "Fluid";

	cout << endl;
	cout << "********************Fluid on GPU Information********************" << endl;
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

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	initFramebuffer();
	initShader();
	initParticle();

	m_hash = vector<int>(info::HASH_SIZE, -1);
	m_neighbors = vector<int>(m_particles.size() * m_params.max_num_neighbors, -1);
	setHash(m_hash, m_neighbors);

	cout << "Number of particles : " << m_particles.size() << endl;
	cout << "********************Fluid on GPU end********************\n" << endl;
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

	cout << "Box Size: " << box_x << " " << box_y << " " << box_z << endl;

	if (m_mesh == nullptr)
	{
		vector<glm::vec3> box;
		for (float x = -box_x; x <= box_x; x += box_x * 2)
		{
			for (float y = 0; y <= box_y; y += box_y)
			{
				for (float z = -box_z; z <= box_z; z += box_z * 2)
				{
					glm::vec3 pos = glm::vec3(x, y, z);
					box.push_back(pos);
				}
			}
		}
		
		vector<glm::vec3> vertices = {
			// Front
			box.at(1),
			box.at(5),
			box.at(7),
			box.at(3),

			// Left
			box.at(0),
			box.at(1),
			box.at(3),
			box.at(2),

			// Right
			box.at(5),
			box.at(4),
			box.at(6),
			box.at(7),

			// Back
			box.at(4),
			box.at(0),
			box.at(2),
			box.at(6),

			// Top
			box.at(3),
			box.at(7),
			box.at(6),
			box.at(2),

			// Bottom
			box.at(0),
			box.at(4),
			box.at(5),
			box.at(1)
		};

		vector<glm::vec3> normals = {
			// Front
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec3(0.0, 0.0, 1.0),
			glm::vec3(0.0, 0.0, 1.0),

			// Left
			glm::vec3(-1.0, 0.0, 0.0),
			glm::vec3(-1.0, 0.0, 0.0),
			glm::vec3(-1.0, 0.0, 0.0),
			glm::vec3(-1.0, 0.0, 0.0),

			// Right
			glm::vec3(1.0, 0.0, 0.0),
			glm::vec3(1.0, 0.0, 0.0),
			glm::vec3(1.0, 0.0, 0.0),
			glm::vec3(1.0, 0.0, 0.0),

			// Back
			glm::vec3(0.0, 0.0, -1.0),
			glm::vec3(0.0, 0.0, -1.0),
			glm::vec3(0.0, 0.0, -1.0),
			glm::vec3(0.0, 0.0, -1.0),

			// Top
			glm::vec3(0.0, 1.0, 0.0),
			glm::vec3(0.0, 1.0, 0.0),
			glm::vec3(0.0, 1.0, 0.0),
			glm::vec3(0.0, 1.0, 0.0),

			// Bottom
			glm::vec3(0.0, -1.0, 0.0),
			glm::vec3(0.0, -1.0, 0.0),
			glm::vec3(0.0, -1.0, 0.0),
			glm::vec3(0.0, -1.0, 0.0),
		};

		vector<info::VertexLayout> layouts;
		for (int i = 0; i < vertices.size(); ++i)
		{
			info::VertexLayout layout;
			layout.position = vertices[i];
			layout.normal = normals[i];
			layouts.push_back(layout);
		}

		vector<uint> indices = {
			0,1,2, 0,2,3, // Front
			4,5,6, 4,6,7, // Left
			8,9,10, 8,10,11, // Right
			12,13,14, 12,14,15, // Back
			16,17,18, 16,18,19, // back
			20,21,22, 20,22,23
		};
		
		 
		cout << "Make mesh" << endl;
		m_mesh = make_shared<Mesh>("Fluid Boundary");
		cout << "goo" << endl;
		m_mesh->getBuffer().createBuffers(layouts, indices);
		m_mesh->computeBoundingBox();	
	}

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

	m_min_box = m_mesh->getBox().getMin();
	m_max_box = m_mesh->getBox().getMax();

	setScale(m_property.at(2));
	setRotation(m_property.at(1));
	setPosition(m_property.at(0));

	glm::vec4 bmin = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(m_min_box, 1.0);
	glm::vec4 bmax = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(m_max_box, 1.0);

	m_params.min_box = glm::vec3(bmin.x, bmin.y, bmin.z);
	m_params.max_box = glm::vec3(bmax.x, bmax.y, bmax.z);

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
	setRotation(glm::vec3(0.0f));

	if (!m_simulation) return;
	
	setScale(m_property.at(2));
	setPosition(m_property.at(0));

	glm::vec4 bmin = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(m_min_box, 1.0);
	glm::vec4 bmax = *m_mesh->getPosition() * *m_mesh->getScale() * glm::vec4(m_max_box, 1.0);

	m_params.min_box = glm::vec3(bmin.x, bmin.y, bmin.z);
	m_params.max_box = glm::vec3(bmax.x, bmax.y, bmax.z);

	m_params.t = t;

	setParams(&m_params);

	vector<glm::vec3> new_pos(m_particles.size());
	simulateCuda(m_particles.size(), t, new_pos);

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

void SPHSystemCuda::draw(const glm::mat4& P, const glm::mat4& V,
	Light& light, glm::vec3& view_pos, ShadowMap& shadow,
	IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut)
{
	simulate();

	m_shader_render->load();
	glActiveTexture(GL_TEXTURE0);
	m_fb_normal->bindFrameTexture();
	m_shader_render->setInt("map", 0);
	m_screen->getMesh()->draw();

	m_shader->load();
	glm::mat4 shadow_proj = (*shadow.getProj()) * (*shadow.getView());
	m_shader->setMat4("light_matrix", shadow_proj);
	m_shader->setVec3("light_pos", *shadow.getPosition());
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(light);
	m_shader->setInt("preview", 0);

	// Load shadow map as texture
	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shadow.getBuffer().bindFrameTexture();
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	irradiance.getCubemapBuffer()->bindCubemapTexture();
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	prefilter.getCubemapBuffer()->bindCubemapTexture();
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	lut.getFrameBuffer()->bindFrameTexture();
	m_mesh->draw(P, V, *m_shader);

}

void SPHSystemCuda::setupFramebuffer(const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap, Camera& camera)
{
	glViewport(0, 0, m_fb_width, m_fb_height);
	computeDepth(camera.getSP(), V);
	computeCurvature(camera.getP(), V);
	computeNormal(camera.getP(), V, depth, cubemap);
}

// [Screen space rendering] : https://developer.download.nvidia.com/presentations/2010/gdc/Direct3D_Effects.pdf
// [Screen space fluid rendering with curvature flow] : https://dl.acm.org/doi/10.1145/1507149.1507164
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

void SPHSystemCuda::reset()
{
	freeResources();

	m_particles.clear();
	initParticle();
	
	m_hash.clear();
	m_neighbors.clear();
	m_hash = vector<int>(info::HASH_SIZE, -1);
	m_neighbors = vector<int>(m_particles.size() * m_params.max_num_neighbors, -1);
	setHash(m_hash, m_neighbors);
}

void SPHSystemCuda::renderProperty()
{
	bool expand_fluid = ImGui::TreeNode("Fluid");
	if (expand_fluid)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		ImVec2 cell_padding(0.0f, 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
		ImGui::BeginTable("Simulation", 2);
		ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();

		static int clicked_sph = 0;
		if (ImGui::Button("Click to simulate"))
		{
			clicked_sph++;
		}

		if (clicked_sph & 1)
		{
			ImGui::TableNextColumn();
			if (ImGui::Button("Start") && m_simulation == false)
			{
				m_simulation = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop") && m_simulation == true)
			{
				m_simulation = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{
				m_simulation = false;
				reset();
			}

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::TableNextColumn();
			ImGui::Text("Speed");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##t", &t, 0.0f, 0.1f, "%.4f", 0);

			ImGui::TableNextColumn();
			ImGui::Text("Gas Constant");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##K", &m_params.K, 0.0f, 10.0f, "%.3f", 0);

			ImGui::TableNextColumn();
			ImGui::Text("Rest Density");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##rDENSITY", &m_params.rDENSITY, 0.0f, 1000.0f, "%.3f", 0);

			ImGui::TableNextColumn();
			ImGui::Text("Viscousity");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##VISC", &m_params.VISC, -1.0f, 10.0f, "%.3f", 0);

			ImGui::TableNextColumn();
			ImGui::Text("Wall Damping");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##WALL", &m_params.WALL, -1.0f, 0.0f, "%.3f", 0);

			ImGui::TableNextColumn();
			ImGui::Text("Render Type");
			ImGui::TableNextColumn();
			ImGui::SliderInt("##TYPE", &render_type, 0, 1);

			ImGui::TableNextColumn();
			ImGui::Text("Iteration Rendering");
			ImGui::TableNextColumn();
			ImGui::SliderInt("##ITERATION", &iteration, 1, 100);
		}

		ImGui::EndTable();

		ImGui::Text("Simulation average: %.3f ms/frame (%.1f FPS)", double(1000.0 / (ImGui::GetIO().Framerate)), double(ImGui::GetIO().Framerate));

		ImGui::PopStyleVar();
		ImGui::TreePop();
	}
}
