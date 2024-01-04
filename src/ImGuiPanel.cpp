#include "ImguiPanel.h"

#include "Buffer.h"
#include "Cloth.h"
#include "Geometry.h"
#include "MarchingCube.h"
#include "Object.h"
#include "ObjectCollection.h"
#include "SPHSystemCuda.h"
#include "SoftBodySolver.h"
#include "Terrain.h"
#include "MeshImporter.h"
#include "Material.h"

ImGuiPanel::ImGuiPanel(string name) :
	m_scene_min(ImVec2(0.0, 0.0)), m_scene_max(ImVec2(0.0, 0.0)), m_name(name) {}

ImGuiPanel::~ImGuiPanel() {}

void ImGuiPanel::calculatePanelSize()
{
	m_scene_min = ImGui::GetWindowContentRegionMin();
	m_scene_max = ImGui::GetWindowContentRegionMax();

	m_scene_min.x += ImGui::GetWindowPos().x;
	m_scene_min.y += ImGui::GetWindowPos().y;
	m_scene_max.x += ImGui::GetWindowPos().x;
	m_scene_max.y += ImGui::GetWindowPos().y;
}

bool ImGuiPanel::mouseInPanel(int x, int y)
{
	if (m_scene_min.x == 0 && m_scene_max.y == 0)
		return false;

	if (x < m_scene_min.x|| y < m_scene_min.y-5)
		return false;

	if (x > m_scene_max.x+5 || y > m_scene_max.y)
		return false;

	return true;
}

ImGuiMenuBar::ImGuiMenuBar() :
	ImGuiPanel("MenuBar"), m_fd(nullptr) {}

ImGuiMenuBar::~ImGuiMenuBar() {}

void ImGuiMenuBar::render(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	ImGui::SeparatorText("Primitives");
	if (ImGui::MenuItem("Cube"))
	{
		shared_ptr<Mesh> mesh;
		shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Cube.txt");
		importer->importMesh(mesh);
		shared_ptr<Object> object = make_shared<Object>(mesh);
		object->setObjectId(scene_objects.size());
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Plane"))
	{
		shared_ptr<Mesh> mesh;
		shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Plane.txt");
		importer->importMesh(mesh);
		shared_ptr<Object> object = make_shared<Object>(mesh);
		object->setObjectId(scene_objects.size());
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}
	
	if (ImGui::MenuItem("Floor"))
	{
		shared_ptr<Mesh> mesh;
		shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Floor.txt");
		importer->importMesh(mesh);
		shared_ptr<Object> object = make_shared<Object>(mesh);
		object->setObjectId(scene_objects.size());
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Sphere"))
	{
		shared_ptr<Object> sphere = make_shared<Sphere>(true);
		sphere->setObjectId(scene_objects.size());
		scene_objects.push_back(sphere);
		addObject(scene_objects, *sphere);
	}
	
	if (ImGui::MenuItem("Metaball"))
	{
		shared_ptr<Object> metaball = make_shared<Metaball>(1.0f);
		metaball->setObjectId(scene_objects.size());
		scene_objects.push_back(metaball);
		addObject(scene_objects, *metaball);
	}

	ImGui::SeparatorText("Landscape");
	if (ImGui::MenuItem("Terrain"))
	{
		shared_ptr<Object> terrain = make_shared<Terrain>(20.0f);
		terrain->setObjectId(scene_objects.size());
		scene_objects.push_back(terrain);
		addObject(scene_objects, *terrain);
	}
	
	ImGui::SeparatorText("Physics Simulator");
	if (ImGui::MenuItem("Cloth"))
	{
		shared_ptr<Object> cloth = make_shared<Cloth>();
		cloth->setObjectId(scene_objects.size());
		scene_objects.push_back(cloth);
		addObject(scene_objects, *cloth);
	}

	if (ImGui::MenuItem("Fluid"))
	{
		shared_ptr<SPHSystemCuda> fluid = make_shared<SPHSystemCuda>(64.0f, 32.0f, 64.0f);
		fluid->setObjectId(scene_objects.size());
		scene_objects.push_back(fluid);
		addObject(scene_objects, *fluid);
	}

	ImGui::Separator();
	if (ImGui::MenuItem("Import..."))
	{
		m_fd = make_unique<FileDialog>();
		string path = m_fd->OpenFile(".fbx");
		if (path != "")
		{
			vector<shared_ptr<Mesh>> meshes;
			shared_ptr<MeshImporter> importer = MeshImporter::create(path);
			importer->importMesh(meshes);

			for (const auto& it : meshes)
			{
				shared_ptr<Object> object = make_shared<Object>(it);
				object->setObjectId(scene_objects.size());
				scene_objects.push_back(object);
				addObject(scene_objects, *object);
			}
		}
	}
}

void ImGuiMenuBar::addObject(
	const vector<shared_ptr<Object>>& scene_objects, Object& add_object)
{
	string name = add_object.getName();
	int count = 0;
	for (auto& it : scene_objects)
	{
		if (it->getName() == name) count++;
	}

	if (count != 0)
	{
		add_object.setId(count);
	}
}

SceneHierarchyPanel::SceneHierarchyPanel() :
	ImGuiPanel("SceneHierarchy")
{}

SceneHierarchyPanel::~SceneHierarchyPanel()
{}

void SceneHierarchyPanel::render(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& active_object)
{
	string name_active = "";	
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();		

		static int n_objects = scene_objects.size();
		static int selection_object = (1 << n_objects);
		int change_size = scene_objects.size();
		if (change_size != n_objects)
		{
			selection_object = (1 << change_size);
			n_objects = change_size;
			cout << "add " << "n_objects: " << n_objects << " with " << selection_object << endl;
		}

		if (active_object)
		{
			name_active = active_object->getIdName();
		}
		else
		{
			selection_object = 1 << n_objects;
		}

		//cout << selection_object << endl;

		collection->renderPanel(active_object, n_objects, selection_object);

		if (scene_objects.size() == 0)
		{
			ImGui::End();
			return;
		}

		int object_clicked = -1;
		bool is_click_item = false;
		for (int i = 0; i < n_objects; ++i)
		{
			string name = scene_objects.at(i)->getIdName();

			if (scene_objects.at(i)->getCollectionId() != -1)
			{
				if ((selection_object & (1 << i)) != 0)
				{
					object_clicked = i;
				}
				continue;
			}
			
			const bool is_selected = (selection_object & (1 << i)) != 0;
			ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
			if (is_selected)
				ImGui::TreeNodeEx((void*)(intptr_t)i, base_flags | ImGuiTreeNodeFlags_Selected, name.c_str());
			else
				ImGui::TreeNodeEx((void*)(intptr_t)i, base_flags, name.c_str());

			if (name == name_active && object_clicked == -1) object_clicked = i;

			if (ImGui::IsItemClicked())
			{			
				if (is_selected)
				{
					object_clicked = -1;
					is_click_item = false;
				}
				else
				{
					object_clicked = i;
					is_click_item = true;
				}
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
				ImGui::Text(name.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::TreePop();
			
			if (object_clicked != -1)
			{
				if (ImGui::GetIO().KeyCtrl)
					selection_object ^= (1 << object_clicked);          // CTRL+click to toggle
				else
					selection_object = (1 << object_clicked);           // Click to single-select
				
				active_object = scene_objects.at(object_clicked);
			}
		}
		
		if (object_clicked == -1)
		{
			active_object = nullptr;
			selection_object = (1 << n_objects);
		}
	}
	ImGui::End();
}

PropertyPanel::PropertyPanel() :
	ImGuiPanel("Property"),
	m_preview_fb(make_unique<FrameBuffer>()), m_preview_object(make_unique<Sphere>(false))
{
	m_preview_fb->createBuffers(256, 256);
}

PropertyPanel::~PropertyPanel()
{}

void PropertyPanel::render(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	ImGui::Begin("Property");
	{
		calculatePanelSize();

		if (clicked_object == nullptr)
		{
			ImGui::End();
			return;
		}

		string name = clicked_object->getIdName();
		const char* object_name = name.c_str();
		
		// Transform panel
		if (ImGui::CollapsingHeader("Transform"))
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
			ImVec2 cell_padding(0.0f, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
			ImGui::BeginTable("Transform", 4);
			ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
			
			clicked_object->renderTransformProperty();

			ImGui::EndTable();
			ImGui::PopStyleVar();
		}

		// Material Panel
		if (ImGui::CollapsingHeader("Material"))
		{	
			clicked_object->renderMeshProperty(*m_preview_object, *m_preview_fb);

			//ImGui::TreePop();
		}


	}
	ImGui::End();
}

PopupObject::PopupObject() : ImGuiPanel("PopupObject") {}

PopupObject::~PopupObject() {}

void PopupObject::popup(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object,
	bool& is_popup, bool& is_clicked_gizmo)
{
	if (clicked_object)
	{
		if (clicked_object->getIsPopup())
		{			
			if (ImGui::GetIO().MouseClicked[1])
				ImGui::OpenPopup("popup");

			if (ImGui::BeginPopup("popup"))
			{
				is_popup = true;
				is_clicked_gizmo = false;

				render(collection, scene_objects, clicked_object);

				ImGui::EndPopup();
			}
			else
			{
				clicked_object->setIsPopup(false);
			}
		}
	}
}

void PopupObject::render(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects,
	shared_ptr<Object>& clicked_object)
{
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Delete"))
		{
			cout << "Delete" << endl;
			clicked_object->setIsDelete(true);
			clicked_object = nullptr;
		}
		ImGui::EndMenu();
	}

	ImGui::SeparatorText("Physics");
}

PopupSceneHierarchy::PopupSceneHierarchy() : ImGuiPanel("PopupScene")
{
}

PopupSceneHierarchy::~PopupSceneHierarchy() {}

void PopupSceneHierarchy::popup(
	shared_ptr<ObjectCollection>& collection, 
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	if (ImGui::GetIO().MouseClicked[1])
		ImGui::OpenPopup("popup2");

	if (ImGui::BeginPopup("popup2"))
	{
		render(collection, scene_objects, clicked_object);

		ImGui::EndPopup();
	}
}

void PopupSceneHierarchy::render(
	shared_ptr<ObjectCollection>& collection,
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	ImGui::Text("Scene Hierarchy Edit Menu");
	ImGui::Separator();
	if (ImGui::MenuItem("New Collection"))
	{
		cout << "Create a new collection" << endl;
		shared_ptr<ObjectCollection> new_collection = make_shared<ObjectCollection>(static_cast<int>(collection->getNumChilds()));
		new_collection->setParent(collection);
		collection->addChild(new_collection);
	}
	
	if (clicked_object)
	{
		if (ImGui::MenuItem("Delete"))
		{
			cout << "Delete" << endl;
			clicked_object->setIsDelete(true);
			clicked_object = nullptr;
		}

		if (ImGui::BeginMenu("Move"))
		{
			collection->renderPopup(clicked_object);
			ImGui::EndMenu();
		}
	}

}
