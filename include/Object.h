#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Map.h"
#include "Quaternion.h"
#include "SoftBodySolver.h"
#include "Camera.h"

using namespace std;

class Object
{
public:
	Object();
	Object(const string& mesh_path);
	Object(const string& mesh_path, const vector<string>& shader);
	~Object();

	virtual void addSoftBodySolver() { m_soft = make_shared<SoftBodySolver>(m_mesh.get()); };
	virtual bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos);
	virtual inline void resetRayHit() { m_mesh->setRayHitMin(FLT_MAX); };

	virtual SoftBodySolver* getSoftBodySolver() { return m_soft.get(); }
	virtual inline shared_ptr<Shader> getShader() { return m_shader; };
	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
	virtual inline bool getIsClick() { return m_click; };
	virtual inline bool getIsDelete() { return m_delete; };
	virtual inline string getName() { return m_name; };
	virtual inline int getId() { return m_id; };
	virtual inline string getIdName()
	{
		string id = "";
		if (m_id)
		{
			id = to_string(m_id);
		}

		return (m_name + id);
	}
	virtual inline string getPath() { return m_path; };
	virtual inline glm::vec3* getProperty(int index) { return &m_property[index]; };
	
	//virtual void setSoftBodySolver(shared_ptr<SoftBodySolver>& soft) { m_soft = soft; };
	virtual void setProperty(int index, glm::vec3 t);
	virtual void setPosition(glm::vec3 pos);
	virtual void setRotation(glm::vec3 rot);
	virtual void setScale(glm::vec3 scale);
	virtual inline void setIsClick(bool click) { m_click = click; };
	virtual inline void setIsDelete(bool d) { m_delete = d; };
	virtual inline void setName(string& name) { m_name = name; };
	virtual inline void setId(int id) { m_id = id; };
		
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

class Outline
{
public:
	Outline(int width, int height);
	~Outline();
	void setupBuffers(GameObject& go, const glm::mat4& V, float width, float height);
	void draw(GameObject& go);

	inline GLuint getOutlineFrame() { return m_outline_buffers.back()->getTextureID(); };
	void clearOutlineFrame();

private:
	vector<unique_ptr<FrameBuffer>> m_outline_buffers;
	unique_ptr<Object> m_debug;
	unique_ptr<Shader> m_mask_shader;
	unique_ptr<Shader> m_outline_shader;
};

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

class GameObject : public Object
{
public:
	GameObject();
	GameObject(const string& mesh_path);
	GameObject(const string& mesh_path, const vector<string>& shader_path);
	~GameObject();

	virtual void move(Camera& camera);
	virtual void drawPreview(Material& mat);
	virtual void drawPreview(vector<shared_ptr<Texture>>& tex);
	virtual void draw(const glm::mat4& P, const glm::mat4& V,
					Light& light, glm::vec3& view_pos, ShadowMap& shadow,
					IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);
	virtual void drawInstance(glm::mat4& P, glm::mat4& V);

	// Getter
	virtual inline int getMoveAxis() { return m_move_axis; };
	virtual inline bool getIsPopup() { return m_is_popup; };

	// Setter
	virtual inline void setMoveAxis(int axis) { m_move_axis = axis; };
	virtual inline void setIsPopup(bool p) { m_is_popup = p; };
	virtual inline void setColor(glm::vec3 color) { m_color = color; };

	virtual void renderProperty() { return; };

private:
	glm::vec3 m_color;
	bool m_is_popup;
	int m_move_axis;
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

class Point
{
public:
	Point(const vector<info::VertexLayout>& layouts);
	~Point();

	void drawPoint(const glm::mat4& P, const glm::mat4& V);
	inline ParticleMesh& getMesh() { return *m_mesh; };
	inline Shader& getShader() { return *m_shader; };

private:
	unique_ptr<ParticleMesh> m_mesh;
	unique_ptr<Shader> m_shader;
};

class Sphere : public Geometry
{
public:
	Sphere(bool is_create_gizmo);
	~Sphere();

private:
	virtual vector<info::VertexLayout> calculateVertex();
	virtual vector<unsigned int> calculateIndex();

	float m_division;
	float m_radius;
};

#endif