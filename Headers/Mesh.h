#pragma once

#ifndef MESH_H
#define MESH_H

#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const int max_bone_per_vertex = 4;

using namespace std;

struct Material
{
	string type = "";
	glm::vec3 ambient = { 0.0f, 0.0f, 0.0f };
	glm::vec3 diffuse = { 0.0f, 0.0f, 0.0f };
	glm::vec3 specular = { 0.0f, 0.0f, 0.0f };
	float shininess = 0.0f;
};

// Inspired and modified from https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
class BoundingBox
{
private:
	glm::vec3 m_min;
	glm::vec3 m_max;

public:
	BoundingBox();
	BoundingBox(glm::vec3 min, glm::vec3 max);
	~BoundingBox();

	bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	void draw();
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
	
	glm::vec3 m_center;
	glm::mat4 m_transform;

public:
	Mesh();
	Mesh(const string& path);
	Mesh(string name, shared_ptr<VertexBuffer> buffer,
		 vector<shared_ptr<Texture>> textures,
		 shared_ptr<Material> material);

	void processMesh();

	void draw();
	virtual void draw(Shader& shader);

	void bind() const;
	void unbind() const;

	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);

	virtual inline void setDirectory(string directory) { m_directory = directory; };
	virtual inline void setTransform(glm::mat4& t) 
	{ 
		m_transform += t; 
	};
	
	inline string getName() { return m_name; };
	inline glm::vec3 getCenter() { return m_center; };
	inline glm::mat4 getTransform() { return m_transform; };
};

class FBXMesh : public Mesh
{
private:	
	vector<shared_ptr<Mesh>> m_meshes;
	vector<shared_ptr<Texture>> m_textures_loaded;
	string m_path;

public:
	FBXMesh();
	FBXMesh(const string& path);
	~FBXMesh();

	void processNode(const aiNode* node, const aiScene* scene);
	shared_ptr<VertexBuffer> processBuffer(const aiMesh* mesh, const aiScene* scene, const string& name, mat4& m);
	shared_ptr<Material> processMaterial(const aiMesh* mesh, const aiScene* scene); // Object material handling functions
	vector<shared_ptr<Texture>> processTextures(const aiMesh* mesh, const aiScene* scene);
	vector<shared_ptr<Texture>> loadTexture(shared_ptr<aiMaterial> mat, aiTextureType type, string typeName); 	// Texture handling functions

	virtual void draw(Shader& shader);

	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	
	virtual inline string getPath() { return m_path; };
	virtual inline void setTransform(glm::mat4& t)
	{
		//cout << "Set position in FBX Mesh" << endl;
		for (auto& it : m_meshes)
		{
			it->setTransform(t);
		}
	};


};

mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat);

#endif // !MESH_H
