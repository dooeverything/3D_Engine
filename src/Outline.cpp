#include "Outline.h"

#include "Shader.h"
#include "Object.h"
#include "MeshImporter.h"
#include "ShaderManager.h"
#include "Quad.h"

Outline::Outline(int width, int height)
{
	cout << "Create an outline constructor" << endl;
	vector<string> outline_shader_paths = { "assets/shaders/Outline.vert", "assets/shaders/Outline.frag" };
	ShaderManager::createShader("Outline", outline_shader_paths);

	vector<string> mask_shader_paths = { "assets/shaders/Mask.vert", "assets/shaders/Mask.frag" };
	ShaderManager::createShader("Mask", mask_shader_paths);

	for (int i = 0; i < 4; ++i)
	{
		m_outline_buffers.push_back(make_unique<FrameBuffer>());
		m_outline_buffers.at(i)->createBuffers(width, height);
	}
	
	vector<string> debug_shader = { "assets/shaders/Debug.vert", "assets/shaders/Debug.frag" };
	ShaderManager::createShader("Debug", debug_shader);

	cout << "Outline finish loading" << endl;
	cout << endl;
}

Outline::~Outline()
{}

void Outline::setupBuffers(Object& go, const glm::mat4& V, float width, float height)
{
	//if (go.getMesh() == nullptr) return;

	float aspect = width / height;
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	glViewport(0, 0, m_outline_buffers[0]->getWidth(), m_outline_buffers[0]->getHeight());

	shared_ptr<Shader> shader = ShaderManager::getShader("Mask");
	m_outline_buffers.at(0)->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->load();
	go.drawMesh(P, V, *shader);
	m_outline_buffers.at(0)->unbind();

	shader = ShaderManager::getShader("Debug");
	//m_debug->setProperty(2, go.getMesh()->getSize());
	m_outline_buffers.at(1)->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(0)->bindFrameTexture();
	//m_debug->getMesh()->draw();
	Quad::getQuad()->draw();
	m_outline_buffers.at(1)->unbind();

	shader = ShaderManager::getShader("Outline");
	m_outline_buffers.at(2)->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(1)->bindFrameTexture();
	shader->setInt("outline_map", 0);
	shader->setFloat("width", 1400.0f);
	shader->setFloat("height", 800.0f);
	shader->setInt("pass", 0);
	shader->setFloat("jump", 1.0f);
	//m_debug->getMesh()->draw();
	Quad::getQuad()->draw();
	m_outline_buffers.at(2)->unbind();

	m_outline_buffers.at(3)->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(2)->bindFrameTexture();
	shader->setInt("outline_map", 0);
	shader->setFloat("width", 1400.0f);
	shader->setFloat("height", 800.0f);
	shader->setInt("pass", 0);
	shader->setFloat("jump", 2.0f);
	//m_debug->getMesh()->draw();
	Quad::getQuad()->draw();
	m_outline_buffers.at(3)->unbind();
}

void Outline::draw(Object& go)
{
	glDisable(GL_DEPTH_TEST);

	shared_ptr<Shader> shader = ShaderManager::getShader("Outline");
	shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.back()->bindFrameTexture();
	shader->setInt("outline_map", 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_outline_buffers.at(0)->bindFrameTexture();
	shader->setInt("mask_map", 1);
	shader->setInt("pass", 2);
	Quad::getQuad()->draw();

	glEnable(GL_DEPTH_TEST);
}

void Outline::clearOutlineFrame()
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Outline");
	m_outline_buffers.back()->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->load();
	shader->setInt("pass", 3);
	Quad::getQuad()->draw();
	m_outline_buffers.back()->unbind();
}