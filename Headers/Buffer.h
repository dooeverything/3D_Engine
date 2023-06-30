#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <SDL.h>
#include <GL/glew.h>
#include "Utils.h"

class VertexBuffer
{
private:
	enum attribs
	{
		POS_ATTRIB = 0,
		NORMAL_ATTRIB = 1,
		TANGENT_ATTRIB = 2,
		TEXCOORD_ATTRIB = 3,
		BONE_ATTRIB = 4,
		BONE_WEIGHT_ATTRIB = 5,
	};

	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_EBO;

	vector<info::VertexLayout> m_layouts;
	unsigned int n_layouts;
	unsigned int n_indices;

public:
	VertexBuffer();
	void createBuffers(const vector<info::VertexLayout>& layouts, const vector<unsigned int>& indices);
	virtual void bind() const;
	virtual void unbind() const;
	
	inline vector<info::VertexLayout> getLayouts() { return m_layouts; };
	inline unsigned int getSizeOfIndices() { return n_indices; };
};

class FrameBuffer
{
protected:
	GLuint m_FBO;
	GLuint m_RBO;
	GLuint m_framebuffer_texture;
public:

	FrameBuffer();
	virtual void createBuffers(int width, int height);
	virtual void bind();
	virtual void unbind();
	void rescaleFrame(int width, int height);
	virtual void bindFrameTexture();

	virtual GLuint& getTextureID() { return m_framebuffer_texture; };
};

class ShadowBuffer : protected FrameBuffer
{
private:
	GLuint m_shadow_map;

public:
	ShadowBuffer();
	~ShadowBuffer();

	virtual void createBuffers(int width, int height);
	virtual void bind();
	virtual void unbind();
	virtual void bindFrameTexture();
	virtual GLuint& getTextureID() { return m_shadow_map; };
};

#endif