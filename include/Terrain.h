#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include <unordered_map>

#include "Object.h"
#include "Tri.h"
#include "ImGuiButton.h"

struct TerrainVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	shared_ptr<TerrainVertex> next;

	TerrainVertex(glm::vec3 p, glm::vec3 c) 
		: pos(p), color(c), next(nullptr) {};
};

class Terrain : public Object
{
public:
	Terrain(float res);
	~Terrain();

	inline void resetHitPos() { m_hit = glm::vec3(-1000.0f); };
	void createVertex();
	void editTerrain(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down);

	virtual void draw(
		const glm::mat4& P,
		const glm::mat4& V,
		const glm::vec3& view_pos,
		const Light& light) override;

	virtual void renderExtraProperty() override;

private:
	int getIndex(const glm::vec2&);

	unique_ptr<ImGuiButton> m_button_plus;
	unique_ptr<ImGuiButton> m_button_minus;

	vector<shared_ptr<Tri>> m_trimeshes;
	vector<shared_ptr<TerrainVertex>> m_vertices;

	glm::vec3 m_hit;

	float m_res;
	float m_width;
	float m_height;
    float m_frequency;
    float m_brush_size;
    float m_strength;

	int m_noise_scale;
    int m_octaves;
    bool m_is_edit;
};

#endif