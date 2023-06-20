#pragma once

#include <string>
#include <glm/glm.hpp>

using namespace std;

class Light
{
public:
	Light();
	Light(glm::vec3 direction, glm::vec3 ambient, 
		  glm::vec3 diffuse, glm::vec3 specular);

	// Getter	
	inline glm::vec3 getDir() { return m_direction; };
	inline glm::vec3 getAmb() { return m_ambient; };
	inline glm::vec3 getDiff() { return m_diffuse; };
	inline glm::vec3 getSpec() { return m_specular; };

	// Setter
	inline void SetDir(glm::vec3 direction) { m_direction = direction; };

	~Light();

private:
	glm::vec3 m_direction;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
};

class PointLight : public Light
{
public:
	PointLight();

	PointLight(glm::vec3 ambient, glm::vec3 diffusion, 
			   glm::vec3 specular, glm::vec3 position,
			   float constant, float linear, float quadratic,
			   glm::vec3 direction = { 0.0f, 0.0f, 0.0f });
	~PointLight();


	// Getter
	inline glm::vec3 getPos() { return m_position; };

	inline float getConst() { return m_constant; };
	inline float getLinear() { return m_linear; };
	inline float getQuad() { return m_quadratic; };

	// Setter
	inline void SetPos(glm::vec3 position) { m_position = position; };

private:
	glm::vec3 m_position;

	float m_constant;
	float m_linear;
	float m_quadratic;
};