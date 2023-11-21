#include "GameObject.h"
#include "Light.h"

GameObject::GameObject() :
	Object(), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path) :
	Object(mesh_path), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path, const vector<string>& shader_path) :
	Object(mesh_path, shader_path), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)), m_move_axis(-1)
{}

GameObject::~GameObject()
{}

void GameObject::move(Camera& camera)
{
	int final_x = 0;
	int final_y = 0;
	float moveCellSize = 0.1f;
	SDL_GetRelativeMouseState(&final_x, &final_y);

	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	if (m_move_axis == 0)
	{
		if (camera.getForward().z <= 0)
		{
			if (final_x < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_x > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
	}
	else if (m_move_axis == 1)
	{
		if (final_y < -1.0)
			pos[m_move_axis] = moveCellSize;
		else if (final_y > 1.0)
			pos[m_move_axis] = -moveCellSize;
	}
	else
	{
		if (camera.getForward().x >= 0.85f)
		{
			if (final_x < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else if (camera.getForward().x <= -0.9f)
		{
			if (final_x > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else if (camera.getForward().z < 0)
		{
			if (final_y < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_y > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_y > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_y < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
	}
	m_property.at(0) += pos;
	setPosition(m_property.at(0));
}

void GameObject::drawPreview(const Material& mat)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	m_shader->load();
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(*light);
	m_shader->setMaterial(mat);
	m_shader->setPVM(P, V, M);
	m_shader->setInt("type", 3);

	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	if (mat.getTexture() != nullptr)
	{
		m_shader->setInt("type", 1);
		m_shader->setInt("texture_map", 4);
		glActiveTexture(GL_TEXTURE0 + 4);
		mat.getTexture()->setActive();
	}
	m_mesh->draw();

	glActiveTexture(GL_TEXTURE0);
}

void GameObject::drawPreview(const vector<shared_ptr<Texture>>& tex)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	m_shader->load();
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(*light);
	m_shader->setPVM(P, V, M);

	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	m_shader->setInt("type", 1);
	m_shader->setInt("texture_map", 4);
	glActiveTexture(GL_TEXTURE0 + 4);
	tex[0]->setActive();

	m_mesh->draw();

	glActiveTexture(GL_TEXTURE0);
}

void GameObject::draw(const glm::mat4& P, const glm::mat4& V,
	Light& light, glm::vec3& view_pos, ShadowMap& shadow,
	IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut)
{
	m_shader->load();
	glm::mat4 shadow_proj = (*shadow.getProj()) * (*shadow.getView());
	m_shader->setMat4("light_matrix", shadow_proj);
	m_shader->setVec3("light_pos", *shadow.getPosition());
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(light);
	m_shader->setInt("preview", 0);

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

	if (m_mesh->getBuffer().getIndices().size() > 1)
		m_mesh->draw(P, V, *m_shader);
	else
		m_mesh->draw(P, V, *m_shader, true);

	//if (m_soft != nullptr)
	//	m_soft->m_tet_mesh->draw(P, V, *m_shader, true);
}

void GameObject::drawInstance(glm::mat4& P, glm::mat4& V)
{
	m_mesh->drawInstance(P, V);
}