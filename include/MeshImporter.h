#pragma once

#ifndef MESHIMPORTER_H
#define MESHIMPORTER_H

#include <cassert>
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include "Utils.h"

using namespace std;

class Material;
class Mesh;
class Texture;
class VertexBuffer;

class MeshImporter
{
public:
	MeshImporter();
	~MeshImporter();

	static shared_ptr<MeshImporter> create(const string& path);
	string findName(const string& path);

	virtual void importMesh(shared_ptr<Mesh>& mesh) = 0;
	virtual void importMesh(vector<shared_ptr<Mesh>>& meshes) = 0;
};

class TXTImporter : public MeshImporter
{
public:
	TXTImporter(const string& path);
	~TXTImporter();

	virtual void importMesh(shared_ptr<Mesh>& mesh) override;
	virtual void importMesh(vector<shared_ptr<Mesh>>& meshes) override;

private:
	void readFile();
	string m_path;
	vector<info::VertexLayout> m_layouts;
	vector<info::uint> m_indices;
};

class FBXImporter : public MeshImporter
{
public:
	FBXImporter(const string& path);
	~FBXImporter();

	virtual void importMesh(shared_ptr<Mesh>& mesh) override;
	virtual void importMesh(vector<shared_ptr<Mesh>>& meshes) override;
private:
	void processNode(const aiNode* node, const aiScene* scene, vector<shared_ptr<Mesh>>& meshes);
	shared_ptr<VertexBuffer> processBuffer(const aiMesh* mesh, const aiScene* scene, const string& name, const glm::mat4& m);
	shared_ptr<Material> processMaterial(const aiMesh* mesh, const aiScene* scene); // Object material handling functions
	void processTextures(const aiMesh* mesh, const aiScene* scene, Material* material);
	vector<shared_ptr<Texture>> loadTexture(shared_ptr<aiMaterial> mat, aiTextureType type, string typeName); 	// Texture handling functions
	glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat);

	vector<shared_ptr<Texture>> m_textures_loaded;

	const aiScene* m_aiScene;
	Assimp::Importer m_importer;

	string m_path;
	string m_directory;
};

#endif // !MESHIMPORTER_H
