#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Map.h"
#include "Quaternion.h"

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
	virtual void setProperty(int index, glm::vec3 t);
	virtual inline void setIsClick(bool click) { m_click = click; };
	virtual inline void setName(string& name) { m_name = name; };
	virtual inline void setId(int id) { m_id = id; };

private:
	virtual void setPosition(glm::vec3 pos);
	virtual void setRotation(glm::vec3 rot);
	virtual void setScale(glm::vec3 scale);
	
protected:
	shared_ptr<Mesh> m_mesh;
	shared_ptr<Shader> m_shader;
	vector<glm::vec3> m_property;
	string m_name;
	string m_path;
	int m_id;
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

	virtual void drawPreview(Material& mat);
	virtual void drawPreview(vector<shared_ptr<Texture>>& tex);
	virtual void draw(glm::mat4& P, glm::mat4& V, Light& light, 
					  glm::vec3& view_pos, ShadowMap& shadow, 
					  IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);
	virtual void drawInstance(glm::mat4& P, glm::mat4& V, Light& light, glm::vec3& view_pos, glm::vec3& light_pos);
	virtual void drawGizmos(glm::mat4& P, glm::mat4& V, glm::vec3& view_pos);
	virtual void loadMesh();
	virtual bool isGizmoClick(glm::vec3& ray_dir, glm::vec3& ray_pos);

	virtual inline void setColor(glm::vec3 color) { m_color = color; };
	virtual inline shared_ptr<Gizmo> getGizmo(int index) { return m_gizmos[index]; };
	virtual inline unsigned int getIrradiance() { return m_irradiance; };
	virtual inline unsigned int getPrefiler() { return m_prefilter; };
	virtual inline GLuint getLUT() { return m_lut; };
	virtual inline void setIrradiance(unsigned int irradiance) { m_irradiance = irradiance; };
	virtual inline void setPrefiler(unsigned int prefilter) { m_prefilter=prefilter; };
	virtual inline void setLUT(GLuint lut) { m_lut = lut; };
	vector<float> m_weights;

private:
	glm::vec3 m_color;

protected:
	vector<shared_ptr<Gizmo>> m_gizmos;

	unsigned int m_irradiance;
	unsigned int  m_prefilter;
	GLuint m_lut;

public:
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

class Point : public Geometry
{
public:
	Point(vector<info::VertexLayout> layouts);
	~Point();

	void drawPoint(glm::mat4& P, glm::mat4& V);

};

class Sphere : public Geometry
{
public:
	Sphere(bool is_create_gizmo, vector<glm::mat4> matrices = {});
	~Sphere();

private:
	virtual vector<info::VertexLayout> calculateVertex();
	virtual vector<unsigned int> calculateIndex();

	float m_division;
	float m_radius;
};

#endif