#include "Particle.h"
#include "Utils.h"
#include <math.h>


Particle::Particle(glm::vec3 p, float m) :
	 m_position(p), m_velocity(0.0f), m_pressure(0.0f), 
	 m_mass(m), m_density(0.0f), m_time(0.0f), m_life(20)
{}

FluidParticle::FluidParticle(glm::vec3 p) :
	Particle(p, 0.0f), m_next(nullptr),
	m_force(glm::vec3(0.0f)), m_gravity(glm::vec3(0.0f, -9.80, 0.0f))
{
}