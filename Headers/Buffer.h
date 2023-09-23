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
	GLuint m_IBO;

	vector<info::VertexLayout> m_layouts;
	vector<glm::mat4> m_matrices;
	unsigned int n_layouts;
	unsigned int n_indices;

public:
	VertexBuffer();
	void createBuffers(const vector<info::VertexLayout>& layouts);
	void createBuffers(const vector<info::VertexLayout>& layouts, const vector<unsigned int>& indices);
	virtual void bind() const;
	virtual void unbind() const;
	
	inline vector<info::VertexLayout> getLayouts() { return m_layouts; };
	inline unsigned int getSizeOfIndices() { return n_indices; };
	inline unsigned int getSizeOfInstance() { return unsigned int(m_matrices.size()); };

	inline void setMatrices(vector<glm::mat4> ms) { m_matrices = ms; };
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

	virtual GLuint getTextureID() { return m_framebuffer_texture; };
};

class DepthBuffer : public FrameBuffer
{
private:
	GLuint m_depth_map;

public:
	DepthBuffer();
	virtual void createBuffers(int width, int height);
	virtual GLuint getTextureID() { return m_depth_map; };
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
	virtual GLuint getTextureID() { return m_shadow_map; };
};

class CubemapBuffer : protected FrameBuffer
{
public:
	CubemapBuffer();
	~CubemapBuffer();

	virtual void createBuffers(int width, int height, bool mipmap);
	virtual void bind();
	virtual void unbind();
	virtual void bindFrameTexture(int i);
	virtual void bindMipMapTexture(int i, int mip);
	void bindCubemapTexture();
	void bindRenderBuffer(int width, int height);
	virtual inline unsigned int getCubemapTexture() { return m_cubemap; };
private:
	unsigned int m_cubemap;
	unsigned int m_width;
	unsigned int m_height;
};

#endif