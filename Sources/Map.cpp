#include "Map.h"
#include "Object.h"

Map::Map() : 
	m_width(0), m_height(0), m_name(""),
	m_shader(NULL), m_mesh(NULL)
{}

Map::Map(int width, int height, string name) :
	m_width(width), m_height(height), m_name(name),
	m_shader(NULL), m_mesh(NULL)
{}

Map::~Map() {}

ShadowMap::ShadowMap(int width, int height) : 
	Map(width, height, "ShadowMap"), 
	m_shadow_buffer(shared_ptr<ShadowBuffer>()),
	m_proj(glm::mat4(0.0f)), m_view(glm::mat4(0.0f)), m_light_position(glm::vec3(0.0f)),
	m_perspective(false)
{}

ShadowMap::ShadowMap(int width, int height, glm::vec3 position, bool perspective) :
	Map(width, height, "ShadowMap"), m_perspective(perspective)
{
	cout << "Shadow map constructor with " << m_width << ", " << m_height << endl;
	
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	if(perspective)
		m_proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	else
		m_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20.0f);
	
	m_view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0, -1.0, 0.0));

	m_light_position = position;
	vector<string> shader_path = { "Shaders/ShadowMap.vert", "Shaders/ShadowMap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	m_shadow_buffer = make_shared<ShadowBuffer>();
	m_shadow_buffer->createBuffers(m_width, m_height);

	cout << "Shadow map constructor finish loading" << endl;
	cout << endl;
}

ShadowMap::~ShadowMap() {}

void ShadowMap::draw(vector<shared_ptr<GameObject>>& gameobjects)
{
	glViewport(0, 0, m_width, m_height);
	m_shadow_buffer->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	for (auto& it : gameobjects)
	{
		if (it->getMesh() == nullptr) continue;
		
		m_shader->load();
		it->getMesh()->draw(m_proj, m_view, *m_shader);
	}
	m_shadow_buffer->unbind();
}

void ShadowMap::draw(shared_ptr<GameObject>& gameobject)
{
	glViewport(0, 0, m_width, m_height);
	m_shadow_buffer->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_shader->load();
	//gameobject->getMesh()->drawInstance(m_proj, m_view, *m_shader);
	m_shadow_buffer->unbind();
}

CubeMap::CubeMap(int width, int height) : 
	Map(width, height, "CubeMap")
{
	cout << "Cubemap constructor " << m_width << ", " << m_height << endl;

	m_cubemap_buffer = make_shared<CubemapBuffer>();
	m_cubemap_buffer->createBuffers(m_width, m_height, false);

	vector<string> shader_path = { "Shaders/Cubemap.vert", "Shaders/Cubemap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	m_mesh = make_shared<Mesh>(m_name, "Models/Cube.txt");
	m_mesh->processMesh();

	m_hdr_texture = make_shared<Texture>("Textures/sky.hdr");
	m_hdr_texture->loadTexture();

	vector<string> shader_background_path = { "Shaders/Background.vert", "Shaders/Background.frag" };
	m_shader_background = make_unique<Shader>(shader_background_path);
	m_shader_background->processShader();

	cout << "Cubemap constructor succesfully loaded" << endl;
	cout << endl;
}

CubeMap::~CubeMap()
{}

void CubeMap::drawMap()
{
	glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)m_width / (float)m_height, 0.1f, 10.0f);

	vector<glm::mat4> PVs;
	glm::vec3 eye_position = glm::vec3(0.0f);
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Right view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Left view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))); // Up view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))); // Down view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Front view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Back view

	m_shader->load();
	m_shader->setFloat("map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_hdr_texture->setActive();

	glViewport(0, 0, m_width, m_height);
	m_cubemap_buffer->bind();
	for (int i = 0; i < 6; ++i)
	{
		m_shader->setMat4("PV", PVs[i]);
		m_cubemap_buffer->bindFrameTexture(i);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_mesh->draw();
	}
	m_cubemap_buffer->unbind();
}

void CubeMap::draw(const glm::mat4& P, const glm::mat4& V)
{
	glDepthFunc(GL_LEQUAL);
	glm::mat4 PV = P * glm::mat4(glm::mat3(V));
	m_shader_background->load();
	m_shader_background->setMat4("PV", PV);
	m_shader_background->setFloat("map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_cubemap_buffer->bindCubemapTexture();
	m_mesh->draw();
	glDepthFunc(GL_LESS);
}

IrradianceMap::IrradianceMap(int width, int height) : 
	Map(width, height, "IrradianceMap")
{
	cout << "Irradiance map constructor " << m_width << ", " << m_height << endl;

	m_irradiance_buffer = make_shared<CubemapBuffer>();
	m_irradiance_buffer->createBuffers(m_width, m_height, false);

	vector<string> shader_path = { "Shaders/Cubemap.vert", "Shaders/Irradiancemap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	m_mesh = make_shared<Mesh>(m_name, "Models/Cube.txt");
	m_mesh->processMesh();

	vector<string> shader_background_path = { "Shaders/Background.vert", "Shaders/Background.frag" };
	m_shader_background = make_unique<Shader>(shader_background_path);
	m_shader_background->processShader();

	cout << "Irradiancemap constructor succesfully loaded" << endl;
	cout << endl;
}

IrradianceMap::~IrradianceMap() {}

void IrradianceMap::drawMap(CubemapBuffer& cubemap)
{
	glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)m_width / (float)m_height, 0.1f, 10.0f);

	vector<glm::mat4> PVs;
	glm::vec3 eye_position = glm::vec3(0.0f);
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Right view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Left view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))); // Up view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))); // Down view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Front view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Back view

	m_shader->load();
	m_shader->setFloat("map", 0);
	glActiveTexture(GL_TEXTURE0);
	cubemap.bindCubemapTexture();

	glViewport(0, 0, m_width, m_height);
	m_irradiance_buffer->bind();
	for (int i = 0; i < 6; ++i)
	{
		m_shader->setMat4("PV", PVs[i]);
		m_irradiance_buffer->bindFrameTexture(i);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_mesh->draw();
	}
	m_irradiance_buffer->unbind();
}

void IrradianceMap::draw(glm::mat4& P, glm::mat4& V)
{
	glDepthFunc(GL_LEQUAL);
	glm::mat4 PV = P * glm::mat4(glm::mat3(V));
	m_shader_background->load();
	m_shader_background->setMat4("PV", PV);
	m_shader_background->setFloat("map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_irradiance_buffer->bindCubemapTexture();
	m_mesh->draw();
	glDepthFunc(GL_LESS);
}

PrefilterMap::PrefilterMap()
{
}

PrefilterMap::PrefilterMap(int width, int height) :
	Map(width, height, "PrefilterMap")
{
	cout << "Prefilter map constructor " << m_width << ", " << m_height << endl;

	m_prefilter_buffer = make_shared<CubemapBuffer>();
	m_prefilter_buffer->createBuffers(m_width, m_height, true);

	vector<string> shader_path = { "Shaders/Cubemap.vert", "Shaders/PrefilterMap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	m_mesh = make_shared<Mesh>(m_name, "Models/Cube.txt");
	m_mesh->processMesh();

	cout << "Prefilter map constructor successfully loaded" << endl;
	cout << endl;
}

PrefilterMap::~PrefilterMap()
{};

void PrefilterMap::drawMap(CubemapBuffer& cubemap)
{
	glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)m_width / (float)m_height, 0.1f, 10.0f);

	vector<glm::mat4> PVs;
	glm::vec3 eye_position = glm::vec3(0.0f);
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Right view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Left view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))); // Up view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))); // Down view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Front view
	PVs.push_back(P * glm::lookAt(eye_position, eye_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Back view

	m_shader->load();
	m_shader->setFloat("map", 0);
	glActiveTexture(GL_TEXTURE0);
	cubemap.bindCubemapTexture();

	m_prefilter_buffer->bind();
	unsigned int levels = 5;
	for (unsigned int i = 0; i < levels; ++i)
	{
		float roughness = (float)i / (float)(levels - 1);
		unsigned int mip_width = static_cast<unsigned int>(m_width * pow(0.5, i));
		unsigned int mip_height = static_cast<unsigned int>(m_height * pow(0.5, i));

		m_prefilter_buffer->bindRenderBuffer(mip_width, mip_height);
		glViewport(0, 0, mip_width, mip_height);
		m_shader->setFloat("roughness", roughness);
		for (int j = 0; j < 6; ++j)
		{
			m_shader->setMat4("PV", PVs[j]);
			m_prefilter_buffer->bindMipMapTexture(j, i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_mesh->draw();
		}
	}
	m_prefilter_buffer->unbind();
}

LUTMap::LUTMap()
{}

LUTMap::LUTMap(int width, int height) : Map(width, height, "LUTMap")
{
	cout << "LUT map constructor " << m_width << ", " << m_height << endl;
	
	m_fb = make_shared<FrameBuffer>();
	m_fb->createBuffers(width, height);

	vector<string> shader_path = { "Shaders/LUTMap.vert", "Shaders/LUTMap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	m_mesh = make_shared<Mesh>(m_name, "Models/Plane.txt");
	m_mesh->processMesh();

	cout << "LUT map constructor successfully loaded" << endl;
	cout << endl;
}

LUTMap::~LUTMap()
{}

void LUTMap::drawMap()
{
	m_fb->bind();
	glViewport(0, 0, m_width, m_height);
	m_shader->load();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_mesh->draw();
	m_fb->unbind();
}

EnvironmentMap::EnvironmentMap(glm::vec3 position)
{
	cout << "Environment map construct" << endl;

	m_eye_position = position;

	m_cubemap_buffer = make_shared<CubemapBuffer>();
	m_cubemap_buffer->createBuffers(WIDTH, HEIGHT, false);

	vector<string> shader_path = { "Shaders/Environmentmap.vert", "Shaders/Environmentmap.frag", "Shaders/Environmentmap.geom" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();
	cout << "Environment map constructor successfully loaded" << endl;
	cout << endl;
}

EnvironmentMap::~EnvironmentMap()
{}

void EnvironmentMap::draw(vector<shared_ptr<GameObject>>& scene, Light& light)
{
	float near_plane = 1.0f;
	float far_plane = 25.0f;

	glm::mat4 P = glm::perspective(glm::radians(90.0f), (float)WIDTH / (float)HEIGHT, near_plane, far_plane);
	vector<glm::mat4> Vs;
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Right view
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Left view
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))); // Up view
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))); // Down view
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Front view
	Vs.push_back(P * glm::lookAt(m_eye_position, m_eye_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // Back view

	glViewport(0, 0, WIDTH, HEIGHT);
	m_cubemap_buffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shader->load();
	for (int i = 0; i < 6; ++i)
	{
		m_shader->setMat4("viewMatrices[" + to_string(i) + "]", Vs[i]);
	}
	m_shader->setVec3("view_pos", m_eye_position);
	m_shader->setLight(light);
	for (auto& it : scene)
	{
		if (glm::length(m_eye_position - *it->getProperty(0)) < 0.1f)
		{
			//cout << "Skip : " << it->getName() << endl;
			continue;
		}

		it->getMesh()->drawLowQuality(*m_shader);
	}
	m_cubemap_buffer->unbind();
}