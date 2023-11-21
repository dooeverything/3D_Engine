#pragma once
#ifndef OUTLINE_H
#define OUTLINE_H

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "Buffer.h"

using namespace std;

class GameObject;
class Shader;

class Outline
{
public:
	Outline(int width, int height);
	~Outline();
	void setupBuffers(GameObject& go, const glm::mat4& V, float width, float height);
	void draw(GameObject& go);

	inline GLuint getOutlineFrame() { return m_outline_buffers.back()->getTextureID(); };
	void clearOutlineFrame();

private:
	vector<unique_ptr<FrameBuffer>> m_outline_buffers;
	unique_ptr<GameObject> m_debug;
	unique_ptr<Shader> m_mask_shader;
	unique_ptr<Shader> m_outline_shader;
};


#endif // !OUTLINE_H
