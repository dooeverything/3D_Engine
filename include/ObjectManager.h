#pragma once
#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Object.h"
class ObjectCollection;

class ObjectManager
{
public:
	ObjectManager(ObjectManager const&) = delete;
	ObjectManager& operator=(ObjectManager const&) = delete;

	static ObjectManager* getObjectManager();

	inline int getNumObjects() { return m_objects.size(); };
	int getObjectId(const string& name);
	
	void resetObjectIds();

	void removeObject(shared_ptr<ObjectCollection>& collection);
	
	void addObject(const shared_ptr<Object>& object);

	bool checkObjectClick(
		shared_ptr<Object>& clicked_object,
		const glm::vec3& ray_dir, const glm::vec3& ray_pos);
	
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
	unordered_map<string, vector<int>> m_object_ids;

	static unique_ptr<ObjectManager> m_object_manager;
	ObjectManager();
};

#endif // !OBJECTMANAGER_H
