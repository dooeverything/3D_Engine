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
	Particle(glm::vec3);
	glm::vec3 m_position;
	glm::vec3 m_velocity;

	float m_time;
	int m_life;
};

class FluidParticle : public Particle
{
public:
	FluidParticle(glm::vec3);
	FluidParticle* m_next;

	glm::vec3 m_force;
	glm::vec3 m_gravity;
	
	float m_density;
	float m_pressure;
};

#endif