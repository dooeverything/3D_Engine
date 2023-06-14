#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>


using namespace std;

namespace info
{
	struct VertexLayout
	{
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
		glm::vec3 tangent = { 0.0f, 0.0f, 0.0f };
		glm::vec2 texCoord = { 0.0f, 0.0f };
	};
}