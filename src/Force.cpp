#include "Force.h"

Force::Force()
{}

Gravity::Gravity(glm::vec3 g) : m_gravity(g)
{}

Pressure::Pressure(glm::vec3 p) : m_pressure(p)
{}

Viscous::Viscous(glm::vec3 v) : m_viscous(v)
{
}
