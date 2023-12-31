#pragma once
#ifndef MESH_H
#define MESH_H

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Buffer.h"
#include "BoundingBox.h"

using namespace std;

class Shader;
class Material;
class Sphere;

class Mesh
{
public:
	Mesh(string name);
	Mesh(string name, 
		 shared_ptr<VertexBuffer> buffer, 
		 shared_ptr<Material> material);

	void draw(const Shader& shader);
	void draw();
	void drawArrays();
	void drawInstance();
	void drawTerrain(const Shader& shader, float res);

	void renderProperty(Sphere& preview_object, const FrameBuffer& preview_fb);

	inline bool intersect(glm::vec3& ray_dir, glm::vec3& ray_pos) { return m_bbox->intersect(ray_dir, ray_pos); };
	inline void updateBuffer(const vector<info::VertexLayout>& layouts) { m_buffer->updateBuffer(layouts); };
	inline void setupBuffer(const vector<info::VertexLayout>& vertices) { m_buffer->createBuffers(vertices); };
	inline void setupBuffer(
		const vector<info::VertexLayout>& vertices,
		const vector<info::uint>& indices) {
		m_buffer->createBuffers(vertices, indices);
	};

	inline void setName(string name) { m_name = name; };
	inline void setRayHitMin(float t) { m_bbox->setRayHitMin(t); }
	inline void setMaterial(const shared_ptr<Material>& material) { m_material = material; };

	inline glm::vec3 getMin() { return m_bbox->getMin(); };
	inline glm::vec3 getMax() { return m_bbox->getMax(); };

	inline glm::vec3 getCenter() { return m_bbox->getCenter(); };
	inline glm::vec3 getSize() { return m_bbox->getSize(); };

	inline shared_ptr<Material> getMaterial() { return m_material; };
	inline string getName() { return m_name; };
	
	inline vector<info::VertexLayout> getVertices() { return m_buffer->getLayouts(); };
	inline vector<info::uint> getIndices() { return m_buffer->getIndices(); };
	inline int getSizeVertices() { return static_cast<int>(m_buffer->getLayouts().size()); };
	inline int getSizeIndices() { return static_cast<int>(m_buffer->getSizeOfIndices()); };
	virtual inline float getRayHitMin() { return m_bbox->getRayHitMin(); };

	void computeBBox(const glm::mat4& M);
private:
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
	void drawInstance();

private:
	unique_ptr<VertexBuffer> m_buffer;
};

#endif // !MESH_H

