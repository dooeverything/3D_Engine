#pragma once
#ifndef MESH_H
#define MESH_H

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Buffer.h"
#include "Shader.h"
#include "BoundingBox.h"

const int max_bone_per_vertex = 4;

using namespace std;

class Texture;
//class Material;

class Mesh
{
public:
	Mesh(string name);
	Mesh(string name, 
		 shared_ptr<VertexBuffer> buffer, 
		 vector<shared_ptr<Texture>> textures,
		 shared_ptr<Material> material);

	void draw();
	virtual void draw(
		const glm::mat4& P, const glm::mat4& V, const glm::mat4& M,
		Shader& shader, bool terrain=false);
	void drawTerrain(const glm::mat4& P, const glm::mat4& V, const glm::mat4& M, 
		Shader& shader, float res);
	void drawInstance(glm::mat4& P, glm::mat4& V);
	void drawArrays();

	//virtual void drawLowQuality(Shader& shader);
	inline bool intersect(glm::vec3& ray_dir, glm::vec3& ray_pos) { return m_bbox->intersect(ray_dir, ray_pos); };
	inline void updateBuffer(const vector<info::VertexLayout>& layouts) { m_buffer->updateBuffer(layouts); };
	inline void setupBuffer(const vector<info::VertexLayout>& vertices) { m_buffer->createBuffers(vertices); };
	inline void setupBuffer(
		const vector<info::VertexLayout>& vertices,
		const vector<info::uint>& indices) {
		m_buffer->createBuffers(vertices, indices);
	};

	inline void setName(string name) { m_name = name; };
	//virtual inline void setDirectory(string directory) { m_directory = directory; };
	inline void setRayHitMin(float t) { m_bbox->setRayHitMin(t); }

	inline glm::vec3 getMin() { return m_bbox->getMin(); };
	inline glm::vec3 getMax() { return m_bbox->getMax(); };

	inline glm::vec3 getCenter() { return m_bbox->getCenter(); };
	inline glm::vec3 getSize() { return m_bbox->getSize(); };

	inline shared_ptr<Material> getMaterial() { return m_material; };
	inline string getName() { return m_name; };
	inline vector<shared_ptr<Texture>>& getTexture() { return m_textures; };
	
	inline vector<info::VertexLayout> getVertices() { return m_buffer->getLayouts(); };
	inline vector<info::uint> getIndices() { return m_buffer->getIndices(); };
	inline int getSizeVertices() { return static_cast<int>(m_buffer->getLayouts().size()); };
	inline int getSizeIndices() { return static_cast<int>(m_buffer->getSizeOfIndices()); };
	virtual inline float getRayHitMin() { return m_bbox->getRayHitMin(); };
	//virtual inline VertexBuffer& getBuffer() { return *m_buffer.get(); };

	void computeBBox(const glm::mat4& M);
private:

	vector<shared_ptr<Texture>> m_textures;
	shared_ptr<VertexBuffer> m_buffer;
	shared_ptr<Material> m_material;
	shared_ptr<BoundingBox> m_bbox;

	string m_name;
};

class ParticleMesh
{
public:
	ParticleMesh(const vector<info::VertexLayout>& layouts);
	~ParticleMesh();
	
	void updateBuffer(vector<info::VertexLayout> layouts);
	VertexBuffer& getBuffer() { return *m_buffer; };
	void drawInstance(const glm::mat4& P, const glm::mat4& V);

private:
	unique_ptr<VertexBuffer> m_buffer;
};

#endif // !MESH_H

