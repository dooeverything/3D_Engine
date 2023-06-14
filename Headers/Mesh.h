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
	glm::mat4 m_transform;

public:
	Mesh();
	Mesh(string name, shared_ptr<VertexBuffer> buffer,
		 vector<shared_ptr<Texture>> textures,
		 shared_ptr<Material> material);

	void loadTXT(const string& file_path);

	void draw();
	virtual void draw(Shader& shader);

	void bind() const;
	void unbind() const;

	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);	
	inline virtual void setDirectory(string directory) { m_directory = directory; };
	
	virtual inline void setPosition(glm::mat4 transform) {m_transform = transform; };
	
	inline string getName() { return m_name; };
};

class FBXMesh : public  Mesh
{
private:	
	vector<shared_ptr<Mesh>> m_meshes;
	vector<shared_ptr<Texture>> m_textures_loaded;

public:
	FBXMesh();
	~FBXMesh();

	void processNode(const aiNode* node, const aiScene* scene);
	shared_ptr<VertexBuffer> processBuffer(const aiMesh* mesh, const aiScene* scene, const string& name, mat4& m);
	shared_ptr<Material> processMaterial(const aiMesh* mesh, const aiScene* scene); // Object material handling functions
	vector<shared_ptr<Texture>> processTextures(const aiMesh* mesh, const aiScene* scene);
	vector<shared_ptr<Texture>> loadTexture(shared_ptr<aiMaterial> mat, aiTextureType type, string typeName); 	// Texture handling functions

	virtual void draw(Shader& shader);

	virtual bool intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos);

	virtual inline void setPosition(glm::mat4 transform)
	{
		//cout << "Set position in FBX Mesh" << endl;
		for (auto& it : m_meshes)
		{
			it->setPosition(transform);
		}
	};

};

mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat);

#endif // !MESH_H
