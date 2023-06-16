#pragma once

#ifndef FILELOADER_H
#define FILELOADER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

using namespace std;

class Object
{
private:

public:
	Object();
	Object(const vector<string>& shader);
	~Object();

	virtual void draw(glm::mat4 P, glm::mat4 V) = 0;
	virtual void loadObject();
	virtual void loadMesh() = 0;
	virtual void loadShader();
	virtual void isPicking(int w, int h, glm::vec3 ray_dir, glm::vec3 ray_pos) = 0;

	virtual shared_ptr<Mesh> getMesh() = 0;
	virtual inline shared_ptr<Shader> getShader() { return m_shader; };

protected:
	shared_ptr<Shader> m_shader;
	bool m_click;
};

class Gizmo : public Object
{
private:
	vector<shared_ptr<Mesh>> m_axis_mesh;
	shared_ptr<Object> m_root;
	int m_clicked_axis;

public:
	Gizmo();
	Gizmo(shared_ptr<Object> root);
	~Gizmo();

	virtual void draw(glm::mat4 P, glm::mat4 V);
	virtual void loadMesh();
	virtual void isPicking(int w, int h, glm::vec3 ray_dir, glm::vec3 ray_pos);
	inline shared_ptr<Mesh> getMesh(int index) { return m_axis_mesh.at(index); };
};

class GameObject : public Object
{
private:
	shared_ptr<Mesh> m_mesh;
	unique_ptr<Gizmo> m_gizmo;

public:
	GameObject();
	GameObject(string& mesh_path, const vector<string>& shader_path);
	~GameObject();

	virtual void draw(glm::mat4 P, glm::mat4 V);
	virtual void loadMesh();
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
};

class FBXObject : public Object
{
private:
	shared_ptr<FBXMesh> m_mesh;
	const aiScene* m_aiScene;
	Assimp::Importer m_importer;

public:
	FBXObject();
	FBXObject(const string& mesh_path,
			  const vector<string>& shader_path);
	~FBXObject();

	virtual void draw(glm::mat4 P, glm::mat4 V);
	virtual void loadMesh();
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
};

#endif