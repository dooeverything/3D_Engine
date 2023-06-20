#include "Light.h"

Light::Light() 
	: m_direction(0.0f), m_ambient(0.0f), 
	  m_diffuse(0.0f), m_specular(0.0f)
{}

Light::Light(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
	: m_direction(direction), m_ambient(ambient), 
	  m_diffuse(diffuse), m_specular(specular)
{}

Light::~Light() {}

PointLight::PointLight() 
	: m_position(0.0f), m_constant(0.0f), 
	  m_linear(0.0f), m_quadratic(0.0f)
{}

PointLight::PointLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
					   glm::vec3 position, float constant, float linear, float quadratic, glm::vec3 direction)
			: Light(ambient, diffuse, specular, direction),
			  m_position(position), m_constant(constant), m_linear(linear), m_quadratic(quadratic)
{}

PointLight::~PointLight()
{}
