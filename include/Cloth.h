#pragma once

#include <iostream>
#include <unordered_map>

#include "Mesh.h"
#include "Object.h"
#include "Particle.h"

using namespace std;

class Cloth : public GameObject
{
public:
	Cloth();
	~Cloth();

	virtual void simulate();
	virtual void renderProperty();
	virtual bool getSimulate() { return m_simulate; };
	virtual void setSimulate(bool s) { m_simulate = s; };

	virtual void draw(const glm::mat4& P, const glm::mat4& V,
		Light& light, glm::vec3& view_pos, ShadowMap& shadow,
		IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);


private:
	void initParticles();
	void buildHash(vector<shared_ptr<ClothParticle>>& predict);

	void updateStretch(int index, vector<shared_ptr<ClothParticle>>& predict);
	void updateBending(int index, float rest_angle, vector<ClothParticle*>& predict);
	void updateCollision(vector<shared_ptr<ClothParticle>>& predict);

	glm::ivec3 getGridPos(glm::vec3 pos);
	uint getIndex(glm::ivec3& pos);
	uint getHashIndex(glm::ivec3& pos);

	vector<shared_ptr<ClothParticle>> m_particles;
	unordered_map<uint, ClothParticle*> m_hash;

	float m_scale;
	float m_width;
	float m_height;
	float m_depth;

	float t;
	float n_sub_steps;
	float t_sub;

	float m_rest;

	bool m_simulate;
};