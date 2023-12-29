#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace std;

class Texture
{
public:
	Texture();
	Texture(const string& path);
	Texture(const string& path, const string& type);
	~Texture();

	void loadTexture();
	void setActive();
	void unload();

	inline int getWidth() { return m_width; };
	inline int getHeight() { return m_height; };
	inline int getWidth() const { return m_width; };
	inline int getHeight() const { return m_height; };
	inline unsigned int getTextureID() const { return m_texture_ID; };

	string getPath() const { return m_path; }
	string getType() const { return m_type; }

private:
	unsigned int m_texture_ID;
	int m_width;
	int m_height;

	string m_path;
	string m_type;
};

#endif