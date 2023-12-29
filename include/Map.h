#pragma once

#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"
#include "Texture.h"

class Object;
class Light;
class Mesh;

using namespace std;

class Map
{
public:
	Map();
	Map(int width, int height, string name);
	~Map();

protected:
	int m_width;
	int m_height;
	string m_name;
	shared_ptr<Mesh> m_mesh;
};

class ShadowMap : public Map
{
public:
	ShadowMap(int width, int height);
	ShadowMap(int width, int height, glm::vec3 m_position, bool perspective);
	~ShadowMap();

	void draw(vector<shared_ptr<Object>>& gameobjects);
	void draw(shared_ptr<Object>& gameobject);

	inline ShadowBuffer& getBuffer() const { return *m_shadow_buffer; };
	inline glm::mat4* getProj() { return &m_proj; };
	inline glm::mat4* getView() { return &m_view; };
	inline glm::vec3* getPosition() { return &m_light_position; };

	inline void setProj(const glm::mat4& P) { m_proj = P; };
	inline void setView(const glm::mat4& V) { m_view = V; };

private:
	shared_ptr<ShadowBuffer> m_shadow_buffer;
	glm::mat4 m_proj;
	glm::mat4 m_view;
	glm::vec3 m_light_position;
	bool m_perspective;
};

class CubeMap : public Map
{
public:
	CubeMap(int width, int height);
	~CubeMap();

	void drawMap();
	void draw(const glm::mat4& P, const glm::mat4& V);
	inline shared_ptr<CubemapBuffer> getCubemapBuffer() { return m_cubemap_buffer; };

private:
	shared_ptr<CubemapBuffer> m_cubemap_buffer;
	shared_ptr<Texture> m_hdr_texture;
};

class IrradianceMap : protected Map
{
public:
	IrradianceMap(int width, int height);
	~IrradianceMap();

	void drawMap(CubemapBuffer& cubemap);
	void draw(glm::mat4& P, glm::mat4& V);
	inline shared_ptr<CubemapBuffer> getCubemapBuffer() { return m_irradiance_buffer; };

private:
	shared_ptr<CubemapBuffer> m_irradiance_buffer;
};

class PrefilterMap : public Map
{
public:
	PrefilterMap();
	PrefilterMap(int width, int height);
	~PrefilterMap();

	void drawMap(CubemapBuffer& cubemap);
	//void draw(glm::mat4& P, glm::mat4& V);
	inline shared_ptr<CubemapBuffer> getCubemapBuffer() { return m_prefilter_buffer; };

private:
	shared_ptr<CubemapBuffer> m_prefilter_buffer;
};

class LUTMap : protected Map
{
public:
	LUTMap();
	LUTMap(int width, int height);
	~LUTMap();

	void drawMap();
	inline shared_ptr<FrameBuffer> getFrameBuffer() { return m_fb; };
private:
	shared_ptr<FrameBuffer> m_fb;
};

class EnvironmentMap : public Map
{
public:
	EnvironmentMap(glm::vec3 position);
	~EnvironmentMap();

	void draw(vector<shared_ptr<Object>>& gameobjects, Light& light);
private:
	const unsigned int WIDTH = 512;
	const unsigned int HEIGHT = 512;

	shared_ptr<CubemapBuffer> m_cubemap_buffer;
	glm::vec3 m_eye_position;
};

#endif