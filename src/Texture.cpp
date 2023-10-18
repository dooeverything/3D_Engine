#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::Texture() : 
	m_texture_ID(0), m_width(0), m_height(0), m_path(""), m_type("") {}

Texture::Texture(const string& path) : 
	m_texture_ID(0), m_width(0), m_height(0), m_path(path), m_type("") {}

Texture::Texture(const string& path, const string& type) : 
	m_texture_ID(0), m_width(0), m_height(0),
	m_path(path), m_type(type) 
{}

Texture::~Texture() {}

void Texture::loadTexture()
{
	int n_channel = 0;

	stbi_set_flip_vertically_on_load(true); // Flip the image upside-down
	unsigned char* texture = stbi_load(m_path.c_str(), &m_width, &m_height, &n_channel, 0); // Load the texture file
	cout << "Load texture from " << m_path << " with size: " << m_width << " " << m_height << endl;

	if (!texture)
	{
		cerr << "Texture failed to load at path: " << m_path << endl;
		stbi_image_free(texture);
		assert(0);
	}

	int format = GL_RGB;
	if (n_channel == 4)
	{
		format = GL_RGBA; // if the texture contains alpha value
	}

	glGenTextures(1, &m_texture_ID);
	glBindTexture(GL_TEXTURE_2D, m_texture_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(texture);

	//GLenum errorCode;
	//if ((errorCode = glGetError()) != GL_NO_ERROR)
	//{
	//	cout << endl;
	//	cerr << "***** TEXTURE ERROR FROM " << m_path << " " << m_type << "*****" << endl;
	//	cerr << "Error Type : " << errorCode << endl;
	//	assert(0);
	//}

	// Set texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::setActive()
{
	glBindTexture(GL_TEXTURE_2D, m_texture_ID);
}

void Texture::unload()
{
	glDeleteTextures(1, &m_texture_ID);
}
