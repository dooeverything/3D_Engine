#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "Texture.h"

using namespace std;

class Material
{
public:
	Material();
	~Material();

	void addTexture(const string& path);

	inline shared_ptr<Texture> getTexture() const { return m_texture; };
	inline glm::vec3 getBaseColor() const { return m_base_color; };
	inline float getMetallic() const { return m_metallic; };
	inline float getRoughness() const { return m_roughness; };
	inline float getAO() const { return m_ao; };

	inline void setBaseColor(glm::vec3 color) { m_base_color = color; };
	inline void setMetallic(float metallic) { m_metallic = metallic; };
	inline void setRoughness(float roughness) { m_roughness = roughness; };
private:
	shared_ptr<Texture> m_texture;

	string m_type;
	glm::vec3 m_base_color;
	float m_metallic;
	float m_roughness;
	float m_ao;
};

#endif
