#pragma once

#ifndef PARTICLE_H
#define PARTICLE_H

#include <random>
#include <vector>
#include "Force.h"
using namespace std;

#define PI 3.14159265f
typedef unsigned int uint;

class Particle
{
public:
	Particle(glm::vec3, float);
	float m_mass;
	float m_density;
	float m_pressure;
	glm::vec3 m_position;
	glm::vec3 m_velocity;

protected:
	float m_time;
	int m_life;
};

class FluidParticle : public Particle
{
public:
	FluidParticle(glm::vec3);

	shared_ptr<FluidParticle> m_next;	
	glm::vec3 m_force;
	glm::vec3 m_gravity;
};

#endif