#pragma once

#ifndef OBJECTCOLLECTION_H
#define OBJECTCOLLECTION_H

#include <iostream>
#include <unordered_map>
#include <string>

#include "Utils.h"

using namespace std;

class Object;
class ObjectCollection;
class SoftBodyObject;
class ShadowMap;

class ObjectCollectionManager
{
public:
	static shared_ptr<ObjectCollection> findRoot(const shared_ptr<ObjectCollection>& object);
	static void findAndRemove(shared_ptr<ObjectCollection>& root, const string& active_object);
	
	static void findAndSwap(
		const shared_ptr<ObjectCollection>& collection,
		const shared_ptr<Object>& source,
		const shared_ptr<Object>& target
	);

	static void removeObjectFromCollection(shared_ptr<ObjectCollection>& collection, const string& active_object);

private:
	ObjectCollectionManager() = delete;
};

class ObjectCollection : public enable_shared_from_this<ObjectCollection>
{
public:
	ObjectCollection(int id);
	~ObjectCollection();
	
	bool findAndSwapObject(
		const shared_ptr<Object>& source,
		const shared_ptr<Object>& target
	);

	bool findAndRemoveObject(const string& active_object);
	void addObject(const shared_ptr<Object>& object);
	void addChild(const shared_ptr<ObjectCollection>& object);

	void renderObjectHierarchy(
		shared_ptr<Object>& active_object,
		int n_scene_objects, 
		int& selection_object);

	void renderPanel(
		shared_ptr<Object>& active_object,
		int n_scene_objects,
		int& selection_object);

	void renderPopup(shared_ptr<Object>& active_object);

	void resetObjects();

	inline weak_ptr<ObjectCollection> getParent() { return m_parent; };
	inline string getNameId() const { return m_name + to_string(m_id); };
	inline int getNumChilds() const { return static_cast<int>(m_childs.size()); };
	inline int getNumObjects() { return m_objects.size(); };
	inline int getId() { return m_id; };

	inline void setParent(const shared_ptr<ObjectCollection> parent) { m_parent = parent; };
	inline void setName(const string& name) { m_name = name; };

private:
	weak_ptr<ObjectCollection> m_parent;
	unordered_map<info::uint, shared_ptr<ObjectCollection>> m_childs;

	vector<shared_ptr<Object>> m_objects;

	string m_name;
	int m_id;
	info::uint n_childs;
};

#endif // !OBJECTCOLLECTION_H
