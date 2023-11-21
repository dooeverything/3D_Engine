#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <string>

using namespace std;

class Light
{
public:
	Light();
	Light(glm::vec3 direction, glm::vec3 ambient, 
		  glm::vec3 diffuse, glm::vec3 specular);

	inline glm::vec3 getDir() const { return m_direction; };
	inline glm::vec3 getAmb() const { return m_ambient; };
	inline glm::vec3 getDiff() const { return m_diffuse; };
	inline glm::vec3 getSpec() const { return m_specular; };

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
	PointLight( glm::vec3 direction, glm::vec3 ambient, 
				glm::vec3 diffusion, glm::vec3 specular, 
				glm::vec3 position, float constant, float linear, float quadratic );
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

#endif // !LIGHT_H