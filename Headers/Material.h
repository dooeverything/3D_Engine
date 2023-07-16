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

	inline glm::vec3& getBaseColor() { return m_base_color; };
	inline float getMetallic() { return m_metallic; };
	inline float getRoughness() { return m_roughness; };
	inline shared_ptr<Texture> getTexture() { return m_texture; };

	inline void setBaseColor(glm::vec3 color) { m_base_color = color; };
	inline void setMetallic(float metallic) { m_metallic = metallic; };
	inline void setRoughness(float roughness) { m_roughness = roughness; };

private:
	shared_ptr<Texture> m_texture;

	string m_type;
	glm::vec3 m_base_color;
	float m_metallic;
	float m_roughness;
};

#endif
