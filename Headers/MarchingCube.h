#pragma once

#include <math.h>
#include <limits.h>
#include <random>
#include "Object.h"
#include "FastNoiseLite.h"

using namespace std;

class MarchingCube : public GameObject
{
public:
    MarchingCube(float size);
    ~MarchingCube();

    virtual float calculateGridValue(glm::vec3 gridPoint) = 0;

    virtual glm::vec3 interpolate(glm::vec3 grid1, glm::vec3 grid2, float gridValue1, float gridValue2, float threshold);

    virtual void polygonize(vector<glm::vec3> grids, vector<float> gridValues);

    virtual void draw(glm::mat4& P, glm::mat4& V, Light& light,
                      glm::vec3& view_pos, ShadowMap& shadow,
                      IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);

    virtual void getVertex() = 0;

protected:
    float m_size;
    float m_grid_size;
    float m_threshold;
    glm::vec3 m_center;
    
    vector<glm::vec3> m_vertices;
    vector<glm::vec3> m_normals;
};

class Metaball : public MarchingCube
{
public:
    Metaball(float size);

    virtual float calculateGridValue(glm::vec3 grid_point);
    virtual void getVertex();
};

class Terrain : public MarchingCube
{
public:
    Terrain(float size);
    virtual float calculateGridValue(glm::vec3 grid_point);
    virtual void getVertex();

private:
    float m_frequency;
    int m_octaves;
    int m_noise_scale;
    vector<float> m_weights;
};



