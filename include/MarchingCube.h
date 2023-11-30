#pragma once

#ifndef MARCHINGCUBE_H
#define MARCHINGCUBE_H

#include <limits.h>
#include <math.h>
#include <random>
#include <unordered_map>

#include "FastNoiseLite.h"
#include "GameObject.h"

class MarchingCube : public GameObject
{
public:
    MarchingCube();
    MarchingCube(float size);
    ~MarchingCube();

    virtual void createVertex() = 0;
    virtual void updateVertex();

    virtual glm::vec3 interpolate(glm::vec3, glm::vec3, float, float, float);
    virtual void polygonize(vector<glm::vec3>, vector<float> gridValues);

    inline virtual float getSize() { return m_size; };
    inline virtual float getGridSize() { return m_grid_size; };
    inline virtual float getThreshold() { return m_threshold; };

    inline virtual void setSize(float size) { m_size = size; };
    inline virtual void setGridSize(float grid_size) { m_grid_size = grid_size; };
    inline virtual void setThreshold(float threshold) { m_threshold = threshold; };

protected:
    float m_size;
    float m_grid_size;
    float m_threshold;

    vector<glm::vec3> m_vertices;
    vector<glm::vec3> m_normals;
    vector<float> m_weights;
};

class Metaball : public MarchingCube
{
public:
    Metaball(float size);

    virtual float getGridValue(glm::vec3 grid_point);
    virtual void createVertex();

private:
    glm::vec3 m_center;
};

#endif // !MARCHINGCUBE_H