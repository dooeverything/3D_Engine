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
class ObjectCollectionManager
{
public:
	static shared_ptr<ObjectCollection> findRoot(shared_ptr<ObjectCollection>& object);
	static void findAndRemove(
		shared_ptr<ObjectCollection>& root,
		const string& name,
		int collection_id);
	static void removeObject(shared_ptr<ObjectCollection>& collection, shared_ptr<Object>& object);
};

class ObjectCollection : public enable_shared_from_this<ObjectCollection>
{
public:
	ObjectCollection(int id);
	~ObjectCollection();

	bool findAndRemoveObject(const string& name);
	void addObject(const shared_ptr<Object>& object);
	void addChild(const shared_ptr<ObjectCollection>& object);

	void renderPanel(
		shared_ptr<Object>& active_object,
		int n_scene_objects,
		int& selection_object);

	void renderPopup(shared_ptr<Object>& active_object);

	inline weak_ptr<ObjectCollection> getParent() { return m_parent; };
	inline shared_ptr<Object> getObject(const shared_ptr<Object>& object);
	inline string getNameId() const { return m_name + to_string(m_id); };
	inline int getNumChilds() const { return static_cast<int>(m_childs.size()); };
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
