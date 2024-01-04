#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Camera.h"
#include "Map.h"
#include "Mesh.h"
#include "Quaternion.h"
#include "Transform.h"

class SoftBodySolver;
class Material;
class Sphere;
class FrameBuffer;

using namespace std;

class Object
{
public:
	Object();
	Object(const string& name);
	Object(const shared_ptr<Mesh>& mesh);
	~Object();
	
	virtual void draw(const glm::mat4& P, const glm::mat4& V,
		Light& light, glm::vec3& view_pos, ShadowMap& shadow,
		IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);

	virtual void drawMesh(const glm::mat4& P, const glm::mat4& V, const Shader& shader);
	virtual void drawInstance(glm::mat4& P, glm::mat4& V);

	inline bool isClick(glm::vec3& ray_dir, glm::vec3& ray_pos) {  return m_mesh->intersect(ray_dir, ray_pos); };
	void calcTransform(const glm::vec3& forward);
	void updateTransform(const glm::vec3& t, Transform::Type type);
	inline void updateVertices(const vector<info::VertexLayout>& vertices) { m_mesh->updateBuffer(vertices); };
	inline void setupVertices(const vector<info::VertexLayout>& vertices) { m_mesh->setupBuffer(vertices); };

	void resetRayHit();
	void addSoftBodySolver();
	inline void addMesh(const shared_ptr<Mesh>& mesh) { m_mesh = mesh; computeBBox(); };

	string getIdName();
	inline glm::mat4 getModelTransform() { return m_transform.getModelTransform(); };
	inline glm::vec3 getSize() { return m_mesh->getSize(); };
	inline glm::vec3 getMin() { return m_mesh->getMin(); };
	inline glm::vec3 getMax() { return m_mesh->getMax(); };
	inline glm::vec3 getCenter() { return m_mesh->getCenter(); };
	inline string getName() { return m_mesh->getName(); };
	inline float getRayHitMin() { return m_mesh->getRayHitMin(); };
	inline int getId() { return m_id; };
	inline int getObjectId() { return m_object_id; };
	inline bool getIsClick() { return m_click; };
	inline bool getIsDelete() { return m_delete; };
	Transform::Type getTransformType() { return m_transform_type; };

	inline void setMaterial(const shared_ptr<Material>& material) { m_mesh->setMaterial(material); };
	inline void setName(const string& name) { m_name = name; };
	inline void setObjectId(int id) { m_object_id = id; };
	void setId(int id);
	void setTransformType(int type);
	inline void setIsClick(bool click) { m_click = click; };
	inline void setIsDelete(bool d) { m_delete = d; };
	
	virtual void setupFramebuffer(const glm::mat4& V, 
								  ShadowMap& depth, 
								  CubeMap& cubemap, 
								  Camera& camera);

	// Getter
	inline int getMoveAxis() { return m_move_axis; };
	inline int getCollectionId() const { return m_collection; };
	inline bool getIsPopup() { return m_is_popup; };

	// Setter
	inline void setMoveAxis(int axis) { m_move_axis = axis; };
	inline void setCollectionId(int id) { m_collection = id; };
	inline void setIsPopup(bool p) { m_is_popup = p; };

	virtual void renderExtraProperty() { return; };
	virtual void renderMeshProperty(Sphere& preview_object, const FrameBuffer& preview_fb) { m_mesh->renderProperty(preview_object, preview_fb); };
	virtual void renderTransformProperty() { m_transform.renderProperty(); computeBBox(); };
	virtual void updateVertex(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down) { return; };
	
protected:
	void drawTerrain(const glm::mat4& P, const glm::mat4& V, const Shader& shader, float res);
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

	int m_object_id;

	int m_move_axis;
	int m_collection;

	bool m_click;
	bool m_delete;

	bool m_is_popup;
};
#endif