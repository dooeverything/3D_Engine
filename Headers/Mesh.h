#pragma once

#ifndef MESH_H
#define MESH_H

#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const int max_bone_per_vertex = 4;

using namespace std;

// Inspired and modified from https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
class BoundingBox
{
private:
	glm::vec3 m_min;
	glm::vec3 m_max;

public:
	BoundingBox(glm::vec3 min, glm::vec3 max);
	~BoundingBox();

	bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	void draw();
	inline glm::vec3 getMin() { return m_min; };
	inline glm::vec3 getMax() { return m_max; };
};

class Mesh
{
protected:
	string m_directory;
	shared_ptr<BoundingBox> m_bbox;
	virtual shared_ptr<BoundingBox> computeBoundingBox();

private:
	string m_name;
	shared_ptr<VertexBuffer> m_buffer;
	vector<shared_ptr<Texture>> m_textures;
	shared_ptr<Material> m_material;
	glm::mat4 m_transform_pos;
	glm::mat4 m_transform_rot;
	glm::mat4 m_transform_scale;
	glm::vec3 m_center;

public:
	Mesh();
	Mesh(string name, const string& path);
	Mesh(string name, vector<info::VertexLayout> layouts, 
		 vector<unsigned int> indices);
	Mesh(string name, shared_ptr<VertexBuffer> buffer,
		 vector<shared_ptr<Texture>> textures,
		 shared_ptr<Material> material);

	virtual void processMesh();

	void draw();
	virtual void draw(glm::mat4& P, glm::mat4& V, Shader& shader);
	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);

	inline void setName(string name) { m_name = name; };
	virtual inline void setDirectory(string directory) { m_directory = directory; };
	virtual void setPosition(glm::mat4 t);
	virtual void setRotation(glm::mat4 t);
	virtual void setScale(glm::mat4 t);

	inline shared_ptr<Material> getMaterial() { return m_material; };
	inline shared_ptr<BoundingBox> getBox() { return m_bbox; };
	inline string getName() { return m_name; };
	inline glm::vec3 getCenter() { return m_center; };
	virtual inline glm::mat4 getTransform()
	{
		return (m_transform_pos * m_transform_rot * m_transform_scale);
	};
	virtual inline glm::mat4* getPosition() { return &m_transform_pos; };
	virtual inline glm::mat4* getRotation() { return &m_transform_rot; };
	virtual inline glm::mat4* getScale() { return &m_transform_scale; };
	virtual inline glm::vec3 getSize() { return abs(m_bbox->getMax() - m_bbox->getMin()); };
};

class FBXMesh : public Mesh
{
private:	
	const aiScene* m_aiScene;
	Assimp::Importer m_importer;
	
	vector<shared_ptr<Mesh>> m_meshes;
	vector<shared_ptr<Texture>> m_textures_loaded;
	string m_path;

public:
	FBXMesh();
	FBXMesh(const string& path);
	~FBXMesh();

	virtual void processMesh();
	void processNode(const aiNode* node, const aiScene* scene);
	shared_ptr<VertexBuffer> processBuffer(const aiMesh* mesh, const aiScene* scene, const string& name, glm::mat4& m);
	shared_ptr<Material> processMaterial(const aiMesh* mesh, const aiScene* scene); // Object material handling functions
	vector<shared_ptr<Texture>> processTextures(const aiMesh* mesh, const aiScene* scene);
	vector<shared_ptr<Texture>> loadTexture(shared_ptr<aiMaterial> mat, aiTextureType type, string typeName); 	// Texture handling functions

	virtual void draw(glm::mat4& P, glm::mat4& V, Shader& shader);

	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	
	virtual void setPosition(glm::mat4 t);
	virtual void setRotation(glm::mat4 t);
	virtual void setScale(glm::mat4 t);

	virtual inline string getPath() { return m_path; };
	virtual inline glm::vec3 getSize()
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(FLT_MIN);


		for (auto& it : m_meshes)
		{
			//cout << "Get size of " << it->getName() << endl;
			glm::vec3 box_min = it->getBox()->getMin();
			glm::vec3 box_max = it->getBox()->getMax();
			for (int i = 0; i < 3; ++i)
			{
				if (box_min[i] < min[i])
				{
					min[i] = box_min[i];
				}

				if (box_max[i] > max[i])
				{
					max[i] = box_max[i];
				}
			}
		}
		//cout << "Final: "  << endl;
		return glm::abs(max-min);
	}
	virtual inline glm::mat4* getPosition() { return m_meshes.at(1)->getPosition(); };
	virtual inline glm::mat4* getRotation() { return m_meshes.at(1)->getRotation(); };
	virtual inline glm::mat4* getScale() { return m_meshes.at(1)->getScale(); };

};

glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat);

ostream& operator<<(ostream& os, const glm::vec2& v);
ostream& operator<<(ostream& os, const glm::vec3& v);
ostream& operator<<(ostream& os, const glm::vec4& v);
ostream& operator<<(ostream& os, const glm::mat3& m);
ostream& operator<<(ostream& os, const glm::mat4& m);

#endif // !MESH_H

