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

class FrameBuffer : private VertexBuffer
{
private:
	GLuint m_RBO;
	GLuint m_FBO;
	GLuint m_framebuffer_texture;

public:
	FrameBuffer();
	void createBuffers(int width, int height);
	void bind() const;
	void unbind() const;
	void rescaleFrame(int width, int height);

	GLuint getTextureID() { return m_framebuffer_texture; };

};



#endif