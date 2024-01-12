#include "ObjectManager.h"

#include "SPHSystemCuda.h"
#include "ObjectCollection.h"
#include "Terrain.h"
#include "Cloth.h"
#include "SoftBodyObject.h"

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

	for (const auto& it : m_terrains)
	{
		if (it.lock())
		{
			if (it.lock()->getNameId() == clicked_object->getNameId())
			{
				it.lock()->editTerrain(ray_dir, ray_pos, true);
				break;
			}
		}
	}

	return true;
}

void ObjectManager::setupFluidsFramebuffer(const glm::mat4& SP, const glm::mat4& P, const glm::mat4& V)
{
	for (const auto& it : m_fluids)
	{
		if (it.lock())
		{
			it.lock()->setupFrameBuffer(SP, P, V);
		}
	}
}

void ObjectManager::setSimulation(bool simulate)
{
	for (const auto& it : m_fluids)
	{
		//cout << "Set simulate" << endl;
		if (it.lock())
		{
			it.lock()->setIsSimulate(simulate);
		}
	}

	for (const auto& it : m_clothes)
	{
		if (it.lock())
		{
			it.lock()->setSimulate(simulate);
		}
	}

	for (const auto& it : m_softs)
	{
		if (it.lock())
		{
			it.lock()->setSimulate(simulate);
		}
	}
}

void ObjectManager::swapObject(
	const shared_ptr<ObjectCollection>& collection, 
	shared_ptr<Object>& source, 
	shared_ptr<Object> target)
{
	source->setIsDelete(true);

	ObjectCollectionManager::findAndSwap(collection, source, target);
}

void ObjectManager::removeObject(shared_ptr<ObjectCollection>& collection)
{
	for (const auto& it : m_objects)
	{
		if (it->getIsDelete())
		{
			cout << "Remove Object " << it->getNameId() << endl;
			m_object_ids[it->getName()][it->getId()] = -1;
			ObjectCollectionManager::removeObjectFromCollection(collection, it->getNameId());
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

void ObjectManager::addFluidObject(const shared_ptr<SPHSystemCuda>& fluid)
{
	if (m_fluids.empty()) fluid->setId(0);
	else fluid->setId(m_fluids.size());

	m_fluids.emplace_back(fluid);
}

void ObjectManager::addTerrain(const shared_ptr<Terrain>& terrain)
{
	m_terrains.emplace_back(terrain);
}

void ObjectManager::addCloth(const shared_ptr<Cloth>& cloth)
{
	m_clothes.emplace_back(cloth);
}

void ObjectManager::addSoftBody(const shared_ptr<SoftBodyObject>& soft)
{
	m_softs.emplace_back(soft);
}

void ObjectManager::resetObjects()
{
	for (int i = 0; i < m_objects.size(); ++i)
	{
		m_objects.at(i)->setIsClick(false);
		m_objects.at(i)->resetRayHit();
	}

	for (const auto& it : m_terrains)
	{
		if (it.lock())
		{
			it.lock()->resetHitPos();
		}
	}
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
		if (m_objects.at(i)->getName() == "Fluid Boundary") continue;

		m_objects.at(i)->drawMesh(P, V, shader);
	}
}
