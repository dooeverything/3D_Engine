#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Buffer.h"
#include "Texture.h"

using namespace std;

class Material
{
public:
	Material();
	~Material();

	void addTexture(const string& path);

	inline glm::vec3& getAmbient() { return m_ambient; };
	inline glm::vec3& getDiffuse() { return m_diffuse; };
	inline glm::vec3& getSpecular() { return m_specular; };
	inline float getShininess() { return m_shininess; };
	inline shared_ptr<Texture> getTexture() { return m_texture; };

	inline void setAmbient(glm::vec3 amb) { m_ambient = amb; };
	inline void setDiffuse(glm::vec3 diff) { m_diffuse = diff; };
	inline void setSpecular(glm::vec3 spec) { m_specular = spec; };
	inline void setShininess(float shininess) { m_shininess = shininess; };
private:
	shared_ptr<Texture> m_texture;

	string m_type;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
	float m_shininess;
};

#endif
