#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include <unordered_map>

#include "GameObject.h"
#include "Tri.h"

struct TerrainVertex
{
	glm::vec3 pos;
	glm::vec3 color;
	shared_ptr<TerrainVertex> next;

	TerrainVertex(glm::vec3 p, glm::vec3 c) 
		: pos(p), color(c), next(nullptr) {};
};

class Terrain : public GameObject
{
public:
	Terrain(float res);
	~Terrain();

	void createVertex();
	virtual void updateVertex(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down);

	virtual void draw(const glm::mat4& P, const glm::mat4& V,
		Light& light, glm::vec3& view_pos, ShadowMap& shadow,
		IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);

	virtual void renderProperty();

private:
	int getIndex(const glm::vec2&);

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