#pragma once
#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Object.h"
class ObjectCollection;
class SPHSystemCuda;

class ObjectManager
{
public:
	ObjectManager(ObjectManager const&) = delete;
	ObjectManager& operator=(ObjectManager const&) = delete;

	static ObjectManager* getObjectManager();

	inline int getNumObjects() { return m_objects.size(); };
	int getObjectId(const string& name);
	
	void resetObjects();
	void resetObjectIds();

	void removeObject(shared_ptr<ObjectCollection>& collection);
	
	void addObject(const shared_ptr<Object>& object);
	void addFluidObject(const shared_ptr<SPHSystemCuda>& fluid);

	bool checkObjectClick(
		shared_ptr<Object>& clicked_object,
		const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	
	void setupFluidsFramebuffer(const glm::mat4& SP, const glm::mat4& P, const glm::mat4& V);
	void setSimulation(bool simulate);

	void drawObjects(
		const glm::mat4& P,
		const glm::mat4& V,
		glm::vec3& view_pos,
		const Light& light);
	
	void drawObjectsMesh(
		const glm::mat4& P, 
		const glm::mat4& V, 
		const Shader& shader);

private:
	vector<shared_ptr<Object>> m_objects;
	vector<weak_ptr<SPHSystemCuda>> m_fluids;
	unordered_map<string, vector<int>> m_object_ids;

	static unique_ptr<ObjectManager> m_object_manager;
	ObjectManager();
};

#endif // !OBJECTMANAGER_H
