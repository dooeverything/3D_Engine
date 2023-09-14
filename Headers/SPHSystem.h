#pragma once

#include "MarchingCube.h"

const int TABLE_SIZE = 100000;
class SPHSystem : public MarchingCube
{
public:
    SPHSystem(float size, int width, int height);
    
    uint getHashIndex(glm::vec3);
    int getGridIndex(glm::vec3);
    glm::vec3 getGridPos(int index);
    glm::ivec3 snapToGrid(glm::vec3);
    
    virtual void initParticles();
    void buildHash();

    void update(float);
    void updateWeights();
    void updateDensPress(float);
    void updateForces(float);
    void reset();

    inline bool getSimulate() { return m_simulation; };
    inline ShadowBuffer& getFB() { return *m_fb; };
    inline ShadowBuffer& getBlurXFB() { return *m_fb_blur_x; };
    inline ShadowBuffer& getBlurYFB() { return *m_fb_blur_y; };
    inline ShadowBuffer& getCurvatureFB() { return *m_fb_curvature; };

    inline FrameBuffer& getNormalFB() { return *m_fb_normal; };

    inline void setSimulate(bool s) { m_simulation = s; };    
    inline void setParticleRadius(float h)
    {
        H = h;
        POLY6 = 315.0f / float(64.0f * PI * pow(H, 9));
        SPICKY = -45.0f / float(PI * pow(H, 6));
        SPICKY2 = 45.0f / float(PI * pow(H, 6));
    }

    void draw(glm::mat4& P, glm::mat4& V, Light& light, 
        glm::vec3& view_pos, glm::vec3& light_pos, CubeMap& cubemap);
    
    void setupFrame(glm::mat4& P, glm::mat4& V, CubeMap& cubemap, int width, int height);

    virtual void createVertex();

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
    
    shared_ptr<GameObject> m_screen;

    shared_ptr<ShadowBuffer> m_fb;
    shared_ptr<ShadowBuffer> m_fb_blur_x;
    shared_ptr<ShadowBuffer> m_fb_blur_y;
    shared_ptr<ShadowBuffer> m_fb_curvature;
    shared_ptr<FrameBuffer> m_fb_normal;

    shared_ptr<Shader> m_shader_curvature;
    shared_ptr<Shader> m_shader_curvature_normal;
    shared_ptr<Shader> m_shader_normal;
    shared_ptr<Shader> m_shader_render;

    vector<bool> cell_map;
    bool m_simulation;

    int m_width;
    int m_height;

    void getDepth(glm::mat4& P, glm::mat4& V);
    void blurDepth();
    void getNormal(glm::mat4& P, glm::mat4& V, CubeMap& cubemap);
    void getCurvatureNormal(glm::mat4& P, glm::mat4& V);
};
