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

	virtual inline void bindTexture() = 0;
	virtual inline void bindTexture() const = 0;

protected:
	int m_width;
	int m_height;
	string m_name;
	shared_ptr<Mesh> m_mesh;
};

class DepthMap : public Map
{
public:
	DepthMap(int width, int height);
	DepthMap(int width, int height, glm::vec3 m_position, bool perspective);
	~DepthMap();

	void draw();

	virtual inline void bindTexture() override { m_buffer->bindFrameTexture(); };
	virtual inline void bindTexture() const override { m_buffer->bindFrameTexture(); };

	inline glm::mat4 getProj() { return m_proj; };
	inline glm::mat4 getView() { return m_view; };
	inline glm::vec3 getPosition() { return m_light_position; };

	inline void setProj(const glm::mat4& P) { m_proj = P; };
	inline void setView(const glm::mat4& V) { m_view = V; };

private:
	shared_ptr<ShadowBuffer> m_buffer;
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

	virtual inline void bindTexture() override { m_buffer->bindCubemapTexture(); };
	virtual inline void bindTexture() const override { m_buffer->bindCubemapTexture(); };

	void drawMap();
	void draw(const glm::mat4& P, const glm::mat4& V);

private:
	shared_ptr<CubemapBuffer> m_buffer;
	shared_ptr<Texture> m_hdr_texture;
};

class IrradianceMap : public Map
{
public:
	IrradianceMap(int width, int height);
	~IrradianceMap();

	virtual inline void bindTexture() override { m_buffer->bindCubemapTexture(); };
	virtual inline void bindTexture() const override { m_buffer->bindCubemapTexture(); };

	void drawMap(const CubeMap& cubemap);
	void draw(glm::mat4& P, glm::mat4& V);

private:
	shared_ptr<CubemapBuffer> m_buffer;
};

class PrefilterMap : public Map
{
public:
	PrefilterMap();
	PrefilterMap(int width, int height);
	~PrefilterMap();

	virtual inline void bindTexture() override { m_buffer->bindCubemapTexture(); };
	virtual inline void bindTexture() const override { m_buffer->bindCubemapTexture(); };

	void drawMap(const CubeMap& cubemap);
	//void draw(glm::mat4& P, glm::mat4& V);

private:
	shared_ptr<CubemapBuffer> m_buffer;
};

class LUTMap : public Map
{
public:
	LUTMap();
	LUTMap(int width, int height);
	~LUTMap();

	virtual inline void bindTexture() override { m_buffer->bindFrameTexture(); };
	virtual inline void bindTexture() const override { m_buffer->bindFrameTexture(); };

	void drawMap();

private:
	shared_ptr<FrameBuffer> m_buffer;
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