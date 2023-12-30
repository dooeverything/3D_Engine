#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

class Transform
{
public:
	enum Type
	{
		TRANSLATE = 0,
		SCALE = 1,
		ROTATE = 2,
	};

	Transform() :
		m_t(glm::vec3(0.0f)),
		m_r(glm::vec3(0.0f)),
		m_s(glm::vec3(1.0f)),
		m_translation(glm::mat4(1.0f)),
		m_rotation(glm::mat4(1.0f)),
		m_scale(glm::mat4(1.0f))
	{};

	~Transform() {};

	void setTranslation(const glm::vec3& p);
	void setRotation(const glm::vec3& r);
	void setScale(const glm::vec3& s);

	void updateTranslation();
	void updateRotation();
	void updateScale();

	inline glm::mat4 getPosition() const { return m_translation; };
	inline glm::mat4 getRotation() const { return m_rotation; };
	inline glm::mat4 getScale() const { return m_scale; };
	inline glm::mat4 getModelTransform() const { return m_translation * m_rotation * m_scale; };

	void renderTranslationPanel();
	void renderRotationPanel();
	void renderScalePanel();

private:
	glm::vec3 m_t;
	glm::vec3 m_r;
	glm::vec3 m_s;

	glm::mat4 m_translation;
	glm::mat4 m_rotation;
	glm::mat4 m_scale;
};

