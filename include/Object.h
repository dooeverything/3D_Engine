#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Camera.h"
#include "Map.h"
#include "Mesh.h"
#include "Quaternion.h"
#include "Transform.h"

class SoftBodySolver;

using namespace std;

class Object
{
public:
	Object();
	Object(const string& name);
	Object(const shared_ptr<Mesh>& mesh);
	~Object();
	
	virtual void drawMesh(const glm::mat4& P, const glm::mat4& V, const glm::mat4& M, Shader& shader);
	virtual void drawPreview(const Material& mat);
	virtual void drawPreview(const vector<shared_ptr<Texture>>& tex);
	virtual void draw(const glm::mat4& P, const glm::mat4& V,
		Light& light, glm::vec3& view_pos, ShadowMap& shadow,
		IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);
	virtual void drawInstance(glm::mat4& P, glm::mat4& V);

	inline bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos) {  return m_mesh->intersect(ray_dir, ray_pos); };
	void calcTransform(const glm::vec3& forward);
	void updateTransform(const glm::vec3& t, Transform::Type type);
	inline void updateVertices(const vector<info::VertexLayout>& vertices) { m_mesh->updateBuffer(vertices); };
	inline void setupVertices(const vector<info::VertexLayout>& vertices) { m_mesh->setupBuffer(vertices); };

	void resetRayHit();
	void addSoftBodySolver();
	inline void addMesh(const shared_ptr<Mesh>& mesh) { m_mesh = mesh; computeBBox(); };

	virtual string getIdName();
	virtual inline int getId() { return m_id; };
	virtual inline string getName() { return m_mesh->getName(); };
	
	virtual inline glm::mat4 getModelTransform() { return m_transform.getModelTransform(); };
	virtual inline glm::vec3 getSize() { return m_mesh->getSize(); };
	virtual inline glm::vec3 getMin() { return m_mesh->getMin(); };
	virtual inline glm::vec3 getMax() { return m_mesh->getMax(); };
	inline glm::vec3 getCenter() { return m_mesh->getCenter(); };

	virtual inline shared_ptr<Mesh> getMesh() { return m_mesh; };
	virtual inline bool getIsClick() { return m_click; };
	virtual inline bool getIsDelete() { return m_delete; };
	Transform::Type getTransformType() { return m_transform_type; };

	virtual inline void setIsClick(bool click) { m_click = click; };
	virtual inline void setIsDelete(bool d) { m_delete = d; };
	virtual inline void setName(const string& name) { m_name = name; };
	virtual void setId(int id);
	void setTransformType(int type);
	
	virtual void setupFramebuffer(const glm::mat4& V, 
								  ShadowMap& depth, 
								  CubeMap& cubemap, 
								  Camera& camera);

	// Getter
	virtual inline int getMoveAxis() { return m_move_axis; };
	virtual inline bool getIsPopup() { return m_is_popup; };

	// Setter
	virtual inline void setMoveAxis(int axis) { m_move_axis = axis; };
	virtual inline void setIsPopup(bool p) { m_is_popup = p; };

	virtual void renderProperty() { return; };
	virtual void updateVertex(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down) { return; };
	
protected:
	void drawTerrain(const glm::mat4& P, const glm::mat4& V, Shader& shader, float res);
	inline vector<info::VertexLayout> getVertices() { return m_mesh->getVertices(); };
	inline vector<info::uint> getIndices() { return m_mesh->getIndices(); };
	inline void updateBuffer(const vector<info::VertexLayout>& layouts) { m_mesh->updateBuffer(layouts); };
	void computeBBox();

private:

	Transform m_transform;
	Transform::Type m_transform_type;

	shared_ptr<Mesh> m_mesh;
	
	string m_id_name;
	string m_name;
	int m_id;

	int m_move_axis;

	bool m_click;
	bool m_delete;

	bool m_is_popup;
};
#endif