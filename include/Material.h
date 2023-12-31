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

class Sphere;
class Shader;
class FileDialog;
class FrameBuffer;

class Material
{
public:
	Material();
	~Material();

 	void loadMaterialToShader(const Shader& shader);
	bool hasTexture();
	void addTexture(const string& path);
	void renderProperty(Sphere& preview_object, const FrameBuffer& preview_fb);

	inline void addTextureBase(const vector<shared_ptr<Texture>>& textures) { m_textures_base = textures; };
	inline void addTextureSpecular(const vector<shared_ptr<Texture>>& textures) { m_textures_specular = textures; };
	inline void addTextureNormal(const vector<shared_ptr<Texture>>& textures) { m_textures_normal = textures; };
	inline void addTextureHeight(const vector<shared_ptr<Texture>>& textures) { m_textures_height = textures; };

	inline glm::vec3 getBaseColor() const { return m_base_color; };
	inline float getMetallic() const { return m_metallic; };
	inline float getRoughness() const { return m_roughness; };
	inline float getAO() const { return m_ao; };

	inline void setBaseColor(glm::vec3 color) { m_base_color = color; };
	inline void setMetallic(float metallic) { m_metallic = metallic; };
	inline void setRoughness(float roughness) { m_roughness = roughness; };

private:
	unique_ptr<FileDialog> m_fd;
	vector<shared_ptr<Texture>> m_textures_base;
	vector<shared_ptr<Texture>> m_textures_specular;
	vector<shared_ptr<Texture>> m_textures_normal;
	vector<shared_ptr<Texture>> m_textures_height;

	string m_type;
	glm::vec3 m_base_color;
	float m_metallic;
	float m_roughness;
	float m_ao;
};

#endif
