#include "ObjectCollection.h"
#include "Object.h"
#include "imgui-docking/imgui.h"

ObjectCollection::ObjectCollection(int id) : 
	m_childs({}), m_objects({}), 
	m_name("Collection"), m_id(id), n_childs(0)
{}

ObjectCollection::~ObjectCollection() {}

shared_ptr<ObjectCollection> ObjectCollectionManager::findRoot(shared_ptr<ObjectCollection>& collection)
{
	shared_ptr<ObjectCollection> current = collection;

	shared_ptr<ObjectCollection> root = nullptr;
	if (collection->getParent().lock())
	{
		root = collection->getParent().lock();
	}

	cout << "current: " << current->getNameId() << endl;
	while (root != nullptr)
	{
		cout << "Find root " << endl;
		cout << root->getNameId() << endl;
		
		current = root;
		root = root->getParent().lock();
		
		if (root == nullptr) cout << "ALready root" << endl;
	}

	return current;
}

void ObjectCollectionManager::findAndRemove(shared_ptr<ObjectCollection>& root, const string& name, int collection_id)
{
	if (root != nullptr)
	{
		root->findAndRemoveObject(name);
	}
}

void ObjectCollectionManager::removeObject(
	shared_ptr<ObjectCollection>& collection, 
	shared_ptr<Object>& object)
{
	if (object == nullptr) return;

	cout << "Remove object" << endl;

	// 1. Find a root node, it collection is the root
	shared_ptr<ObjectCollection> root = collection;
	if (collection->getParent().lock() != nullptr)
	{
		root = findRoot(collection);
	}

	// 2. Find a node where object is in, and remove it
	findAndRemove(root, object->getIdName(), object->getCollectionId());

	cout << endl;
}

bool ObjectCollection::findAndRemoveObject(const string& name)
{
	bool is_erase = false;

	int j = 0;
	for (; j < m_objects.size(); ++j)
	{
		if (name == m_objects.at(j)->getIdName())
		{
			is_erase = true;
			break;
		}
	}
	cout << "Find and remove from " << getNameId() << endl;

	if (is_erase)
	{
		m_objects.erase(m_objects.begin() + j);	
		return true;
	}

	for (int i = 0; i < m_childs.size(); ++i)
	{
		if (m_childs.at(i)->findAndRemoveObject(name))
		{
			cout << "successfully removed" << endl;
			return true;
		}
	}

	return false;
}

void ObjectCollection::addObject(const shared_ptr<Object>& object)
{
	for (const auto& it : m_objects)
	{
		if (it->getIdName() == object->getIdName()) return;
	}

	cout << "Add object" << endl;
	m_objects.emplace_back(object);
}

void ObjectCollection::addChild(const shared_ptr<ObjectCollection>& child)
{
	//parent->setParent(shared_from_this());
	m_childs[n_childs] = child;
	n_childs += 1;
	cout << "Add child " << n_childs << endl;
}

void ObjectCollection::renderPanel(
	shared_ptr<Object>& active_object,
	int n_scene_objects,
	int& selection_object)
{
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
									ImGuiTreeNodeFlags_SpanAvailWidth;
	
	bool node_open = false;

	if (!m_parent.lock())
		node_open = true;
	else
		node_open = ImGui::TreeNodeEx(getNameId().c_str());

	if (ImGui::BeginDragDropTarget()) {
		
		if (ImGui::GetIO().MouseReleased[0] && active_object)
		{
			cout << "Move " << active_object->getIdName() << " to " << getNameId() << endl;
			
			ObjectCollectionManager::removeObject(shared_from_this(), active_object);
			active_object->setCollectionId(m_id);
			addObject(active_object);
			active_object = nullptr;
			selection_object = (1 << n_scene_objects);
		}

		ImGui::EndDragDropTarget();
	}
	
	string name_active = "";
	if (active_object)	name_active = active_object->getIdName();

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
		ImGui::Text(getNameId().c_str());
		ImGui::EndDragDropSource();
		node_open = false;
	}

	if (node_open)
	{
		int object_clicked = -1;
		for (int i = 0; i < m_objects.size(); ++i)
		{
			string name = m_objects.at(i)->getIdName();
			const bool is_selected = (selection_object & (1 << m_objects.at(i)->getObjectId())) != 0;			
			ImGuiTreeNodeFlags object_flags = base_flags | ImGuiTreeNodeFlags_Leaf;
			if (is_selected)
				ImGui::TreeNodeEx((void*)(intptr_t)i, object_flags | ImGuiTreeNodeFlags_Selected, name.c_str());
			else
				ImGui::TreeNodeEx((void*)(intptr_t)i, object_flags, name.c_str());

			if (name == name_active && object_clicked == -1)
				object_clicked = m_objects.at(i)->getObjectId();

			if (ImGui::IsItemClicked())
			{
				if (is_selected)
				{
					selection_object = (1 << n_scene_objects);
					object_clicked = -1;
				}
				else
				{
					object_clicked = m_objects.at(i)->getObjectId();
				}
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
				ImGui::Text(name.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::TreePop();

			if (object_clicked != -1 && object_clicked == m_objects.at(i)->getObjectId())
			{
				if (ImGui::GetIO().KeyCtrl)
					selection_object ^= (1 << object_clicked);          // CTRL+click to toggle
				else
					selection_object = (1 << object_clicked);           // Click to single-select

				active_object = m_objects.at(i);
			}
		}

		for (info::uint i = 0; i < n_childs; ++i)
		{
			m_childs.at(i)->renderPanel(active_object, n_scene_objects, selection_object);
		}

		if (m_parent.lock()) ImGui::TreePop();
	
	}

}

void ObjectCollection::renderPopup(shared_ptr<Object>& active_object)
{
	if (!m_parent.lock())
	{
		for (info::uint i = 0; i < n_childs; ++i)
		{
			m_childs.at(i)->renderPopup(active_object);
		}
	}
	else
	{
		if (ImGui::MenuItem(getNameId().c_str()	))
		{
			ObjectCollectionManager::removeObject(shared_from_this(), active_object);
			active_object->setCollectionId(m_id);
			addObject(active_object);
			for (info::uint i = 0; i < n_childs; ++i)
			{
				m_childs.at(i)->renderPopup(active_object);
			}
		}
	}
}

inline shared_ptr<Object> ObjectCollection::getObject(const shared_ptr<Object>& object)
{
	return shared_ptr<Object>();
}




