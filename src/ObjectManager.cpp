#include "ObjectManager.h"

#include "ObjectCollection.h"

ObjectManager::ObjectManager() :
	m_objects({})
{}

ObjectManager* ObjectManager::getObjectManager()
{
	if (m_object_manager == nullptr)
	{
		m_object_manager = unique_ptr<ObjectManager>(new ObjectManager());
	}

	return m_object_manager.get();
}

int ObjectManager::getObjectId(const string& name)
{
	if (m_object_ids.find(name) == m_object_ids.end())
	{
		m_object_ids[name].push_back(0);
		return 0;
	}

	int n_objects = m_object_ids[name].size();
	for (int i = 0; i < n_objects; ++i)
	{
		if (m_object_ids[name][i] == -1)
		{
			m_object_ids[name][i] = i;
			return i;
		}
	}

	m_object_ids[name].push_back(n_objects);
	return n_objects;
}

bool ObjectManager::checkObjectClick(shared_ptr<Object>& clicked_object, const glm::vec3& ray_dir, const glm::vec3& ray_pos)
{
	float min_hit = FLT_MAX;
	for (int i = 0; i < m_objects.size(); ++i)
	{
		if (m_objects[i]->isClick(ray_dir, ray_pos))
		{
			float hit = m_objects[i]->getRayHitMin();
			if (hit < min_hit)
			{
				clicked_object = m_objects[i];
				min_hit = hit;
			}
		}
	}

	if (min_hit >= FLT_MAX)
	{
		return false;
	}

	return true;
}

void ObjectManager::removeObject(shared_ptr<ObjectCollection>& collection)
{
	//cout << "Remove Object" << endl;
	for (const auto& it : m_objects)
	{
		if (it->getIsDelete())
		{
			m_object_ids[it->getName()][it->getId()] = -1;
			ObjectCollectionManager::removeObjectFromCollection(collection, it->getIdName());
		}
	}

	m_objects.erase(
		remove_if(m_objects.begin(), m_objects.end(),
			[](const shared_ptr<Object>& go) {return go->getIsDelete() == true; }
		),
		m_objects.end()
	);
}

void ObjectManager::addObject(const shared_ptr<Object>& object)
{
	int id = getObjectId(object->getName());
	object->setId(id);

	m_objects.emplace_back(object);
}

void ObjectManager::resetObjectIds()
{
	for (int i = 0; i < m_objects.size(); ++i)
	{
		m_objects.at(i)->setObjectId(i);
	}
}

void ObjectManager::drawObjects(
	const glm::mat4& P,
	const glm::mat4& V,
	glm::vec3& view_pos,
	const Light& light)
{
	for (int i = 0; i < m_objects.size(); ++i)
	{
		m_objects.at(i)->draw(P, V, view_pos, light);
	}
}

void ObjectManager::drawObjectsMesh(const glm::mat4& P, const glm::mat4& V, const Shader& shader)
{
	for (int i = 0; i < m_objects.size(); ++i)
	{
		m_objects.at(i)->drawMesh(P, V, shader);
	}
}
