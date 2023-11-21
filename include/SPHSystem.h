#pragma once
#ifndef SPHSYSTEM_H
#define SPHSYSTEM_H

#include <unordered_map>

#include "Camera.h"
#include "GameObject.h"
#include "Particle.h"
#include "Point.h"
//#include "Object.h"

const int TABLE_SIZE = 100000;

class SPHSystem : public GameObject
{
public:
    SPHSystem(float width, float height, float depth);
    
    info::uint getHashIndex(glm::ivec3& pos);
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
        POLY6 = 315.0f / float(64.0f * info::PI * pow(H, 9));
        SPICKY = -45.0f / (info::PI * pow(h, 6));
        SPICKY2 = -SPICKY;
    }
    
    virtual void update();
    virtual void draw();
    
    virtual void setupFrame(const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap, Camera& camera);

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
    float SCALE;
    
    // Parameters for experiment
    float t;
    int iteration;
    int render_type;

private:
    void updateDensPress();
    void updateForces();

    void getDepth(const glm::mat4& P, const glm::mat4& V, Camera& camera);
    void getCurvature(const glm::mat4& P, const glm::mat4& V);
    void getNormal(const glm::mat4& P, const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap);
    void blurDepth();

    vector<shared_ptr<FluidParticle>> m_particles;
    unordered_map<info::uint, FluidParticle*> m_hash_table;
    
    unique_ptr<Point> m_point;

    unique_ptr<GameObject> m_screen;

    unique_ptr<ShadowBuffer> m_fb;
    unique_ptr<ShadowBuffer> m_fb_blur_x;
    unique_ptr<ShadowBuffer> m_fb_blur_y;
    unique_ptr<ShadowBuffer> m_fb_curvature;
    unique_ptr<ShadowBuffer> m_fb_curvature2;
    unique_ptr<FrameBuffer> m_fb_normal;

    unique_ptr<Shader> m_shader_depth;
    unique_ptr<Shader> m_shader_curvature;
    unique_ptr<Shader> m_shader_curvature_normal;
    unique_ptr<Shader> m_shader_normal;
    unique_ptr<Shader> m_shader_render;

    float m_grid_width;
    float m_grid_height;
    float m_grid_depth;

    int m_fb_width;
    int m_fb_height;

    bool m_simulation;
};

#endif // !SPHSYSTEM_H