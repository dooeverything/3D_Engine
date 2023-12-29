#pragma once
#ifndef OUTLINE_H
#define OUTLINE_H

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "Buffer.h"

using namespace std;

class Object;
class Shader;

class Outline
{
public:
	Outline(int width, int height);
	~Outline();
	void setupBuffers(Object& go, const glm::mat4& V, float width, float height);
	void draw(Object& go);

	inline GLuint getOutlineFrame() { return m_outline_buffers.back()->getTextureID(); };
	void clearOutlineFrame();

private:
	vector<unique_ptr<FrameBuffer>> m_outline_buffers;
};


#endif // !OUTLINE_H
