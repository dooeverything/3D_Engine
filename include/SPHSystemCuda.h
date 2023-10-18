#pragma once
#include <unordered_map>

#include "Object.h"
#include "Camera.h"
#include "Particle.h"

// Host

class SPHSystemCuda : public GameObject
{
public:
	SPHSystemCuda(float width, float height, float depth);
	~SPHSystemCuda();
	//void updateHash();

	void simulate();
	virtual void draw();

	virtual void setupFramebuffer(
		const glm::mat4& V, ShadowMap& depth,
		CubeMap& cubemap, Camera& camera);

	//float H;
	//float H2;
	//float POLY6;
	//float SPICKY;
	//float SPICKY2;

	//float MASS;
	//float K;
	//float rDENSITY;
	//float VISC;
	//float WALL;
	//float SCALE;

	// Parameters for experiment
	float t;
	int iteration;
	int render_type;
	bool m_simulation;

	info::SPHParams m_params;

private:
	vector<shared_ptr<FluidParticle>> m_particles;
	vector<int> m_neighbors;
	vector<int> m_hash;
	unique_ptr<Point> m_point;

	unique_ptr<GameObject> m_screen;

	unique_ptr<ShadowBuffer> m_fb;
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

	void initParticle();

	void initFramebuffer();

	void initShader();

	void computeDepth(const glm::mat4& P, const glm::mat4& V);
	
	void computeCurvature(const glm::mat4& P, const glm::mat4& V);
	
	void computeNormal(const glm::mat4& P, const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap);

	//glm::ivec3 getHashPos(const glm::vec3&);
	//uint getHashKey(const glm::ivec3&);
	//void fillHash();
};