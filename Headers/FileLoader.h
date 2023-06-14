#pragma once

#ifndef FILELOADER_H
#define FILELOADER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

using namespace std;

class FileLoader
{
private:

public:
	FileLoader();
	~FileLoader();

	virtual void loadMesh(const string& file_path) = 0;
	virtual shared_ptr<Mesh> getMesh() = 0;
	virtual inline string getPath() { return m_path; };

protected:
	string m_path;
};

class TXTLoader : public FileLoader
{
private:
	shared_ptr<Mesh> m_mesh;

public:
	TXTLoader();
	~TXTLoader();

	virtual void loadMesh(const string& file_path);
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
};

class FBXLoader : public FileLoader
{
private:
	shared_ptr<FBXMesh> m_mesh;
	const aiScene* m_aiScene;
	Assimp::Importer m_importer;

public:
	FBXLoader();
	~FBXLoader();

	virtual void loadMesh(const string& file_path);
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
};

#endif