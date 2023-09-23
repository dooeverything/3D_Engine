#pragma once

#include "MarchingCube.h"

const int TABLE_SIZE = 100000;

class SPHSystem : public GameObject
{
public:
    SPHSystem(float width, float height, float depth);
    
    uint getHashIndex(glm::ivec3& pos);
    glm::ivec3 snapToGrid(glm::vec3);
    
    void initParticles();
    void buildHash();
    void reset();
    void setupFB();
    void setupShader();

    inline virtual bool getSimulate() { return m_simulation; };
    inline ShadowBuffer& getFB() { return *m_fb; };
    inline ShadowBuffer& getBlurXFB() { return *m_fb_blur_x; };
    inline ShadowBuffer& getBlurYFB() { return *m_fb_blur_y; };
    inline ShadowBuffer& getCurvatureFB() { return *m_fb_curvature; };

    inline FrameBuffer& getNormalFB() { return *m_fb_normal; };

    inline void setSimulate(bool s) { m_simulation = s; };    
    inline void setParticleRadius(float h)
    {
        H = h;
        H2 = h * h;
        POLY6 = 315.0f / float(64.0f * PI * pow(H, 9));
        SPICKY = -45.0f / (PI * pow(h, 6));
        SPICKY2 = -SPICKY;
    }


    virtual void update();
    virtual void draw();
    
    virtual void setupFrame(glm::mat4& P, glm::mat4& V, CubeMap& cubemap, int width, int height);

    float H;
    float H2;
    float POLY6;
    float SPICKY;
    float SPICKY2;

    float MASS;
    float K;
    float rDENSITY;
    float VISC;
    float WALL;

    float t;
private:
    vector<shared_ptr<FluidParticle>> m_particles;
    unordered_map<uint, FluidParticle*> m_hash_table;
    unique_ptr<Point> m_point;
    unique_ptr<Sphere> m_sphere;

    
    unique_ptr<GameObject> m_screen;

    unique_ptr<ShadowBuffer> m_fb;
    unique_ptr<ShadowBuffer> m_fb_blur_x;
    unique_ptr<ShadowBuffer> m_fb_blur_y;
    unique_ptr<ShadowBuffer> m_fb_curvature;
    unique_ptr<FrameBuffer> m_fb_normal;

    unique_ptr<Shader> m_shader_depth;
    unique_ptr<Shader> m_shader_curvature;
    unique_ptr<Shader> m_shader_curvature_normal;
    unique_ptr<Shader> m_shader_normal;
    unique_ptr<Shader> m_shader_render;

    bool m_simulation;

    float m_point_size;
    float m_grid_width;
    float m_grid_height;
    float m_grid_depth;

    int m_fb_width;
    int m_fb_height;

    void updateDensPress();
    void updateForces();
    void getDepth(glm::mat4& P, glm::mat4& V);
    void blurDepth();
    void getCurvature(glm::mat4& P, glm::mat4& V);
    void getNormal(glm::mat4& P, glm::mat4& V, CubeMap& cubemap);
};
