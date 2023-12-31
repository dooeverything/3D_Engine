#include "Object.h"

#include "glm/gtx/string_cast.hpp"

#include "Buffer.h"
#include "Material.h"
#include "MeshImporter.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "SoftBodySolver.h"
#include "Geometry.h"
#include "Gizmo.h"
#include "Light.h"

Object::Object() :
	m_mesh(nullptr), m_name(""),
	m_id(0), m_move_axis(-1), m_transform_type(Transform::TRANSLATE),
	m_click(false), m_delete(false), m_is_popup(false) {}

Object::Object(const string& name) :
	m_mesh(nullptr), m_name(name),
	m_id(0), m_move_axis(-1), m_transform_type(Transform::TRANSLATE),
	m_click(false), m_delete(false), m_is_popup(false) {}

Object::Object(const shared_ptr<Mesh>& mesh) :
	m_mesh(mesh), m_name(m_mesh->getName()), 
	m_id(0), m_move_axis(-1), m_transform_type(Transform::TRANSLATE),
	m_click(false), m_delete(false), m_is_popup(false)
{
	computeBBox();
}

Object::~Object()
{}

void Object::drawMesh(
	const glm::mat4& P, const glm::mat4& V,
	const Shader& shader)
{
	shader.load();
	shader.setPVM(P, V, m_transform.getModelTransform());

	m_mesh->draw();
}

void Object::draw(
	const glm::mat4& P, 
	const glm::mat4& V, 
	Light& light, 
	glm::vec3& view_pos, 
	ShadowMap& shadow, IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Default");

	shader->load();
	glm::mat4 shadow_proj = (*shadow.getProj()) * (*shadow.getView());
	shader->setMat4("light_matrix", shadow_proj);
	shader->setVec3("light_pos", *shadow.getPosition());
	shader->setVec3("view_pos", view_pos);
	shader->setLight(light);
	shader->setInt("preview", 0);

	shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shadow.getBuffer().bindFrameTexture();
	shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	irradiance.getCubemapBuffer()->bindCubemapTexture();
	shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	prefilter.getCubemapBuffer()->bindCubemapTexture();
	shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	lut.getFrameBuffer()->bindFrameTexture();

	shader->setPVM(P, V, m_transform.getModelTransform());

	m_mesh->draw(*shader);
}

void Object::drawInstance(glm::mat4& P, glm::mat4& V)
{
	m_mesh->drawInstance();
}

void Object::resetRayHit()
{
	m_mesh->setRayHitMin(FLT_MAX);
}

void Object::addSoftBodySolver()
{
	//m_soft = make_shared<SoftBodySolver>(m_mesh.get());
}

string Object::getIdName()
{
	return m_id_name;
}

void Object::calcTransform(const glm::vec3& forward)
{
	int final_x = 0;
	int final_y = 0;
	float moveCellSize = 0.1f;
	SDL_GetRelativeMouseState(&final_x, &final_y);

	//cout << "Move " << m_move_axis << endl;
	glm::vec3 t = glm::vec3(0.0, 0.0, 0.0);
	if (m_move_axis == 0)
	{
		if (forward.z <= 0)
		{
			if (final_x < -1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				t[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_x > 1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				t[m_move_axis] = moveCellSize;
		}
	}
	else if (m_move_axis == 1)
	{
		if (final_y < -1.0)
			t[m_move_axis] = moveCellSize;
		else if (final_y > 1.0)
			t[m_move_axis] = -moveCellSize;
	}
	else
	{
		if (forward.x >= 0.85f)
		{
			if (final_x < -1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				t[m_move_axis] = moveCellSize;
		}
		else if (forward.x <= -0.9f)
		{
			if (final_x > 1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				t[m_move_axis] = moveCellSize;
		}
		else if (forward.z < 0)
		{
			if (final_y < -1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_y > 1.0)
				t[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_y > 1.0)
				t[m_move_axis] = -moveCellSize;
			else if (final_y < -1.0)
				t[m_move_axis] = moveCellSize;
		}
	}

	updateTransform(t, m_transform_type);
}

void Object::updateTransform(const glm::vec3& t, Transform::Type type)
{
	if (type == Transform::TRANSLATE)
	{
		m_transform.setTranslation(t);
	}
	else if (type == Transform::ROTATE)
	{
		m_transform.setRotation(t);
	}
	else if (type == Transform::SCALE)
	{
		m_transform.setScale(t);
	}

	computeBBox();
}

void Object::setId(int id)
{
	m_id = id;
	string sid = to_string(m_id);
	if (m_id >= 10 && m_id < 100)
	{
		sid = "0" + sid;
	}
	else if (m_id < 10)
	{
		sid = "00" + sid;
	}
	cout << m_id << endl;
	cout << sid << endl;
	m_id_name = m_name + "_" + sid;
}

void Object::setTransformType(int type)
{
	if (type == 0)
	{
		m_transform_type = Transform::TRANSLATE;
	}
	else if (type == 1)
	{
		m_transform_type = Transform::SCALE;
	}
}

void Object::setupFramebuffer(const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap, Camera& camera)
{
	return;
}

void Object::drawTerrain(const glm::mat4& P, const glm::mat4& V, const Shader& shader, float res)
{
	shader.load();
	shader.setPVM(P, V, m_transform.getModelTransform());

	m_mesh->drawTerrain(shader, res);
}

void Object::computeBBox()
{
	//cout << "Computer bbox " << endl;
	//cout << glm::to_string(m_transform.getPosition()) << endl;
	m_mesh->computeBBox(m_transform.getModelTransform());
}
