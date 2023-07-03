#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "imgui-docking/imgui.h"

using namespace std;

class Object
{
public:
	Object();
	Object(const string& mesh_path);
	Object(const string& mesh_path, const vector<string>& shader);
	~Object();

	virtual bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos);

	//virtual inline void isClick(bool click) { m_click = click; };
	virtual inline void loadShader() { m_shader->processShader(); };
	virtual inline shared_ptr<Shader> getShader() { return m_shader; };
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
	virtual inline bool getIsClick() { return m_click; };
	virtual inline string getName() { return m_name; };
	virtual inline glm::vec3* getProperty(int index) { return &m_property[index]; };
	virtual void setProperty(int index, glm::vec3 t);
	virtual inline void setIsClick(bool click) { m_click = click; };
	
private:
	virtual void setPosition(glm::vec3 pos);
	virtual void setRotation(glm::vec3 rot);
	virtual void setScale(glm::vec3 scale);
	
protected:
	shared_ptr<Mesh> m_mesh;
	shared_ptr<Shader> m_shader;
	vector<glm::vec3> m_property;
	string m_name;
	bool m_click;
};

class GameObject;

class Outline
{
public:
	Outline(int width, int height);
	~Outline();
	void setupBuffers(GameObject& go, glm::mat4& P, glm::mat4& V);
	void draw(GameObject& go, glm::mat4& P, glm::mat4& V);

	inline GLuint& getOutlineFrame() { return m_outline_buffers.back()->getTextureID(); };
	void clearOutlineFrame();

private:
	vector<unique_ptr<FrameBuffer>> m_outline_buffers;
	unique_ptr<GameObject> m_debug;
	unique_ptr<Shader> m_mask_shader;
	unique_ptr<Shader> m_outline_shader;
};

class ShadowMap : public Object
{
public:
	ShadowMap();
	ShadowMap(glm::vec3 m_position);
	~ShadowMap();

	void draw(vector<shared_ptr<GameObject>>& gameobjects);
	inline ShadowBuffer& getBuffer() { return *m_shadow_buffer; };
	inline glm::mat4* getProj() { return &m_proj; };
	inline glm::mat4* getView() { return &m_view; };
	inline glm::vec3* getPosition() { return &m_light_position; };

private:
	const unsigned int WIDTH = 2048;
	const unsigned int HEIGHT = 2048;

	shared_ptr<ShadowBuffer> m_shadow_buffer;
	glm::mat4 m_proj;
	glm::mat4 m_view;
	glm::vec3 m_light_position;
};

class Grid : public Object
{
	public:
		Grid();
		void draw(glm::mat4& P, glm::mat4& V, glm::vec3 cam_pos);
};

class Gizmo : public Object
{
public:
	Gizmo(GameObject& root, int axis);
	~Gizmo();

	void draw(glm::mat4& P, glm::mat4& V, glm::mat4& M);

private:
	GameObject& m_root;
	int m_axis;
};

class GameObject : public Object
{
public:
	GameObject();
	GameObject(const string& mesh_path);
	GameObject(const string& mesh_path, const vector<string>& shader_path);
	~GameObject();

	virtual void draw(glm::mat4& P, glm::mat4& V, Light& light, glm::vec3& view_pos, ShadowMap& shadow);
	virtual void drawGizmos(glm::mat4& P, glm::mat4& V, glm::vec3& view_pos);
	virtual void loadMesh();
	virtual bool isGizmoClick(glm::vec3& ray_dir, glm::vec3& ray_pos);

	virtual inline void setColor(glm::vec3 color) { m_color = color; };
	virtual inline shared_ptr<Gizmo> getGizmo(int index) { return m_gizmos[index]; };

private:
	glm::vec3 m_color;

protected:
	vector<shared_ptr<Gizmo>> m_gizmos;

public:
	int m_move_axis;
	shared_ptr<FrameBuffer> m_frame_buffer = nullptr;
	float m_screen_w = 0.0;
	float m_screen_h = 0.0;

};

class Geometry : public GameObject
{
public:
	Geometry();
	~Geometry();

private:
	virtual vector<info::VertexLayout> calculateVertex() { return {}; };
	virtual vector<unsigned int> calculateIndex() { return {}; };
};

class Sphere : public Geometry
{
public:
	Sphere();
	~Sphere();
	
private:
	virtual vector<info::VertexLayout> calculateVertex();
	virtual vector<unsigned int> calculateIndex();

	float m_division;
	float m_radius;
};

#endif