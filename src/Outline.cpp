#include "Outline.h"

#include "Shader.h"
#include "GameObject.h"

Outline::Outline(int width, int height)
{
	cout << "Create an outline constructor" << endl;
	vector<string> outline_shader_paths = { "assets/shaders/Outline.vert", "assets/shaders/Outline.frag" };
	m_outline_shader = make_unique<Shader>(outline_shader_paths);
	m_outline_shader->processShader();

	vector<string> mask_shader_paths = { "assets/shaders/Mask.vert", "assets/shaders/Mask.frag" };
	m_mask_shader = make_unique<Shader>(mask_shader_paths);
	m_mask_shader->processShader();

	for (int i = 0; i < 4; ++i)
	{
		m_outline_buffers.push_back(make_unique<FrameBuffer>());
		m_outline_buffers.at(i)->createBuffers(width, height);
	}

	vector<string> debug_shader = { "assets/shaders/Debug.vert", "assets/shaders/Debug.frag" };
	m_debug = make_unique<GameObject>("assets/models/Debug.txt", debug_shader);
	cout << "Outline finish loading" << endl;
	cout << endl;
}

Outline::~Outline()
{}

void Outline::setupBuffers(GameObject& go, const glm::mat4& V, float width, float height)
{
	if (go.getMesh() == nullptr) return;

	float aspect = width / height;
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	glViewport(0, 0, m_outline_buffers[0]->getWidth(), m_outline_buffers[0]->getHeight());

	m_outline_buffers.at(0)->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_mask_shader->load();
	if (go.getMesh()->getBuffer().getIndices().size() > 1)
	{
		go.getMesh()->draw(P, V, *m_mask_shader);
	}
	else
	{
		go.getMesh()->draw(P, V, *m_mask_shader, true);
	}
	m_outline_buffers.at(0)->unbind();

	m_debug->setProperty(2, go.getMesh()->getSize());
	m_outline_buffers.at(1)->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_debug->getShader()->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(0)->bindFrameTexture();
	m_debug->getMesh()->draw();
	m_outline_buffers.at(1)->unbind();

	m_outline_buffers.at(2)->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_outline_shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(1)->bindFrameTexture();
	m_outline_shader->setInt("outline_map", 0);
	m_outline_shader->setFloat("width", 1400.0f);
	m_outline_shader->setFloat("height", 800.0f);
	m_outline_shader->setInt("pass", 0);
	m_outline_shader->setFloat("jump", 1.0f);
	m_debug->getMesh()->draw();
	m_outline_buffers.at(2)->unbind();

	m_outline_buffers.at(3)->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_outline_shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.at(2)->bindFrameTexture();
	m_outline_shader->setInt("outline_map", 0);
	m_outline_shader->setFloat("width", 1400.0f);
	m_outline_shader->setFloat("height", 800.0f);
	m_outline_shader->setInt("pass", 0);
	m_outline_shader->setFloat("jump", 2.0f);
	m_debug->getMesh()->draw();
	m_outline_buffers.at(3)->unbind();
}

void Outline::draw(GameObject& go)
{
	m_outline_shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.back()->bindFrameTexture();
	m_outline_shader->setInt("outline_map", 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_outline_buffers.at(0)->bindFrameTexture();
	m_outline_shader->setInt("mask_map", 1);
	m_outline_shader->setInt("pass", 2);
	m_debug->getMesh()->draw();
}

void Outline::clearOutlineFrame()
{
	m_outline_buffers.back()->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_outline_shader->load();
	m_outline_shader->setInt("pass", 3);
	m_debug->getMesh()->draw();
	m_outline_buffers.back()->unbind();
}