#include "Material.h"

Material::Material() :
	m_texture(nullptr)
{
	m_type = "";
	m_ambient = { 0.8f, 0.8f, 0.8f };
	m_diffuse = { 0.8f, 0.8f, 0.8f };
	m_specular = { 0.8f, 0.8f, 0.8f };
	m_shininess = 32.0f;
}

Material::~Material()
{}

void Material::addTexture(const string& path)
{
	m_texture = make_shared<Texture>(path);
	m_texture->loadTexture();
}






