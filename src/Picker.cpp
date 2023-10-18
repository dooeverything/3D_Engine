#include "Picker.h"

Picker::Picker() : m_ray_world(0.0f)
{}

Picker::~Picker()
{
}

void Picker::calcualteMouseRay(float width, float height, 
							   int mouse_x, int mouse_y, glm::mat4 view)
{	
	glm::vec2 ray_nds = getNDS(width, height, mouse_x, mouse_y); // Screen space to Normalized Device space
	glm::vec4 ray_clip = glm::vec4(ray_nds, -1.0, 1.0); // ND Space to Clipped space
	
	float aspect = width / height;
	glm::vec4 ray_camera = getCameraSpace(ray_clip, aspect);
	m_ray_world = getWorldSpace(ray_camera, view);
}

glm::vec2 Picker::getNDS(int width, int height, int mouse_x, int mouse_y)
{
	float x = (2.0f * mouse_x) / width - 1.0f;
	float y = 1.0f - (2.0f * mouse_y) / height;
	return glm::vec2(x, y);
}

glm::vec4 Picker::getCameraSpace(glm::vec4 clip_coord, float aspect)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	glm::vec4 ray_cam = glm::inverse(P) * clip_coord;
	return glm::vec4(ray_cam.x, ray_cam.y, -1.0f, 0.0f);
}

glm::vec3 Picker::getWorldSpace(glm::vec4 camera_coord, glm::mat4 view)
{
	glm::vec4 ray = glm::inverse(view) * camera_coord;
	glm::vec3 ray_norm = glm::normalize(glm::vec3(ray.x, ray.y, ray.z));
	return ray_norm;
}