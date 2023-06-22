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
		virtual inline glm::vec3* getPosition() { return &m_position; };
		virtual void setPosition(glm::vec3 pos);
		virtual inline void setIsClick(bool click) { m_click = click; };
	protected:
		shared_ptr<Mesh> m_mesh;
		shared_ptr<Shader> m_shader;
		glm::vec3 m_position;
		string m_name;
		bool m_click;
};

class Grid : public Object
{
	public:
		Grid();
		void draw(glm::mat4& P, glm::mat4& V, glm::vec3 cam_pos);
};

class GameObject;
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

		virtual void draw(glm::mat4& P, glm::mat4& V, Light& light, glm::vec3& view_pos);
		virtual void loadMesh();
		virtual void loadOutlineShader();
		virtual bool isGizmoClick(glm::vec3& ray_dir, glm::vec3& ray_pos);

		virtual inline void setColor(glm::vec3 color) { m_color = color; };
		virtual inline shared_ptr<Gizmo> getGizmo(int index) { return m_gizmos[index]; };

	private:
		shared_ptr<Shader> m_outline_shader;
		glm::vec3 m_color;

	protected:
		vector<shared_ptr<Gizmo>> m_gizmos;

	public:
		int m_move_axis;
		int m_x;
		int m_y;
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