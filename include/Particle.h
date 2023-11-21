#pragma once

#ifndef PARTICLE_H
#define PARTICLE_H

#include <random>
#include <vector>

#include <glm/glm.hpp>

using namespace std;

class Particle
{
public:
	Particle(glm::vec3);
	glm::vec3 m_position;
	glm::vec3 m_velocity;

	glm::vec3 m_force;
	glm::vec3 m_gravity;

	float m_time;
	int m_life;
};

class FluidParticle : public Particle
{
public:
	FluidParticle(glm::vec3);
	FluidParticle* m_next;
	
	float m_density;
	float m_pressure;
	float m_weight;
};

class ClothParticle : public Particle
{
public:
	ClothParticle(glm::vec3);
	ClothParticle* m_next;

	vector<int> m_ids;
	float m_mass;
	bool m_pinned;
};

class SoftParticle : public Particle
{
public:
	SoftParticle(glm::vec3);

	vector<int> m_indices;
	float m_mass;
	bool m_pinned;
};

#endif