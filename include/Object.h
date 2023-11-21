#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Camera.h"
#include "Quaternion.h"

#include "Map.h"
#include "Mesh.h"

class SoftBodySolver;

using namespace std;

class Object
{
public:
	Object();
	Object(const string& mesh_path);
	Object(const string& mesh_path, const vector<string>& shader);
	~Object();

	bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos);
	void resetRayHit();
	void addSoftBodySolver();

	virtual string getIdName();
	virtual inline shared_ptr<SoftBodySolver> getSoftBodySolver() { return m_soft; }
	virtual inline shared_ptr<Shader> getShader() { return m_shader; };
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
	virtual inline bool getIsClick() { return m_click; };
	virtual inline bool getIsDelete() { return m_delete; };
	virtual inline string getName() { return m_name; };
	virtual inline int getId() { return m_id; };
	virtual inline string getPath() { return m_path; };
	virtual inline glm::vec3 getProperty(int index) { return m_property[index]; };
	
	virtual void setProperty(int index, const glm::vec3& t);
	virtual void setPosition(const glm::vec3& pos);
	virtual void setRotation(const glm::vec3& rot);
	virtual void setScale(const glm::vec3& scale);
	virtual inline void setIsClick(bool click) { m_click = click; };
	virtual inline void setIsDelete(bool d) { m_delete = d; };
	virtual inline void setName(const string& name) { m_name = name; };
	virtual inline void setId(int id) { m_id = id; };
	
	virtual void setupFramebuffer(const glm::mat4& V, 
								  ShadowMap& depth, 
								  CubeMap& cubemap, 
								  Camera& camera);

protected:
	shared_ptr<SoftBodySolver> m_soft;
	shared_ptr<Mesh> m_mesh;
	shared_ptr<Shader> m_shader;

	vector<glm::vec3> m_property;
	string m_name;
	string m_path;
	int m_id;
	bool m_click;
	bool m_delete;
};

class GameObject;

class Grid : public Object
{
public:
	Grid();
	void draw(const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos);
};

class Gizmo : public Object
{
public:
	Gizmo(int axis);
	~Gizmo();

	void draw(GameObject& go, const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos);
	virtual bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos);

private:
	int m_axis;
};

#endif