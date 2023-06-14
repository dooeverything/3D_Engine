#pragma once

#include <string>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Light
{
public:
	Light();
	Light(vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular);

	// Getter	
	inline vec3 getDir() { return m_direction; };
	inline vec3 getAmb() { return m_ambient; };
	inline vec3 getDiff() { return m_diffuse; };
	inline vec3 getSpec() { return m_specular; };

	// Setter
	inline void SetDir(vec3 direction) { m_direction = direction; };

	~Light();

private:
	vec3 m_direction;

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;
};

class PointLight : public Light
{
public:
	PointLight();

	PointLight(vec3 ambient, vec3 diffusion, vec3 specular, vec3 position,
				float constant, float linear, float quadratic,
				vec3 direction = { 0.0f, 0.0f, 0.0f });
	~PointLight();


	// Getter
	inline vec3 getPos() { return m_position; };

	inline float getConst() { return m_constant; };
	inline float getLinear() { return m_linear; };
	inline float getQuad() { return m_quadratic; };

	// Setter
	inline void SetPos(vec3 position) { m_position = position; };

private:
	vec3 m_position;

	float m_constant;
	float m_linear;
	float m_quadratic;
};