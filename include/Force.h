#pragma once

#ifndef FORCE_H
#define FORCE_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
using namespace std;

class Force 
{
public:
	Force();
};

class Gravity
{
public:
	Gravity(glm::vec3);
	virtual inline glm::vec3& getGravity() { return m_gravity; };

private:
	glm::vec3 m_gravity;
};

class Pressure : public Force
{
public:
	Pressure(glm::vec3 = glm::vec3(0.0f));
	virtual inline glm::vec3& getPressure() { return m_pressure; };
private:
	glm::vec3 m_pressure;
};

class Viscous : public Force
{
public:
	Viscous(glm::vec3 = glm::vec3(0.0f));
	virtual inline glm::vec3& getViscosity() { return m_viscous; };

private:
	glm::vec3 m_viscous;
};

#endif