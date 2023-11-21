#include "Particle.h"
#include "Utils.h"
#include <math.h>

Particle::Particle(glm::vec3 p) :
	m_position(p), m_velocity(0.0f), m_time(0.0f), m_life(20),
	m_force(glm::vec3(0.0f)), m_gravity(glm::vec3(0.0f, -9.80f, 0.0f))
{}

FluidParticle::FluidParticle(glm::vec3 p) :
	Particle(p), m_next(nullptr),
	m_pressure(0.0f), m_density(0.0f), m_weight(0.0f)
{
}

ClothParticle::ClothParticle(glm::vec3 p) :
	Particle(p), m_next(nullptr), m_ids({}), m_pinned(false) , m_mass(1.0f)
{
}

SoftParticle::SoftParticle(glm::vec3 p) :
	Particle(p), m_indices({}), m_mass(1.0f), m_pinned(false)
{
}