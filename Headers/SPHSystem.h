#pragma once

#include "MarchingCube.h"

const int TABLE_SIZE = 100000;
class SPHSystem : public MarchingCube
{
public:
    SPHSystem(float size);
    uint getHashIndex(glm::vec3);
    int getGridIndex(glm::vec3);
    float getGridValue(int index);
    glm::vec3 getGridPos(int index);
    glm::ivec3 snapToGrid(glm::vec3);
    virtual void initParticles();
    void buildHash();
    virtual void createVertex();

    void update(float);
    void updateWeights();
    void updateDensPress(float);
    void updateForces(float);
    void reset();

    inline bool getSimulate() { return m_simulation; };
    inline FrameBuffer& getFB() { return *m_fb; };
    inline FrameBuffer& getBlurXFB() { return *m_fb_blur_x; };
    inline FrameBuffer& getBlurYFB() { return *m_fb_blur_y; };

    inline FrameBuffer& getNormalFB() { return *m_fb_normal; };

    inline void setSimulate(bool s) { m_simulation = s; };    
    inline void setParticleRadius(float r)
    {
        H = r;
        POLY6 = 315.0f / float(64.0f * PI * pow(H, 9));
        SPICKY = -45.0f / float(PI * pow(H, 6));
        SPICKY2 = 45.0f / float(PI * pow(H, 6));
    }

    void draw(glm::mat4& P, glm::mat4& V, Light& light, 
        glm::vec3& view_pos, glm::vec3& light_pos, CubeMap& cubemap);
    
    void setupFrame(glm::mat4& P, glm::mat4& V, float width, float height, float cam_zoom);

    float H;
    float POLY6;
    float SPICKY;
    float SPICKY2;

    float MASS;
    float K;
    float rDENSITY;
    float VISC;
    float WALL;

    bool m_render_type;

private:
    vector<shared_ptr<FluidParticle>> m_particles;
    unordered_map<uint, shared_ptr<FluidParticle>> m_hash_table;
    shared_ptr<Point> m_point;
    shared_ptr<Sphere> m_sphere;

    shared_ptr<GameObject> m_debug;
    shared_ptr<GameObject> m_debug2;
    shared_ptr<GameObject> m_debug3;

    shared_ptr<FrameBuffer> m_fb;
    shared_ptr<FrameBuffer> m_fb_blur_x;
    shared_ptr<FrameBuffer> m_fb_blur_y;
    shared_ptr<FrameBuffer> m_fb_normal;

    shared_ptr<Shader> m_blur;

    vector<bool> cell_map;
    bool m_simulation;
};
