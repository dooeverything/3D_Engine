#include "ImguiPanel.h"

#include "Buffer.h"
#include "Cloth.h"
#include "Geometry.h"
#include "MarchingCube.h"
#include "Object.h"
#include "SPHSystemCuda.h"
#include "SoftBodySolver.h"
#include "Terrain.h"
#include "MeshImporter.h"
#include "Material.h"

ImGuiPanel::ImGuiPanel(string name) :
	m_scene_min(ImVec2(0.0, 0.0)), m_scene_max(ImVec2(0.0, 0.0)), m_name(name)
{
}

ImGuiPanel::~ImGuiPanel()
{}

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

ImGuiMenuBar::ImGuiMenuBar(string name) :
	ImGuiPanel(name), m_fd(nullptr)
{
}

ImGuiMenuBar::~ImGuiMenuBar()
{}

void ImGuiMenuBar::render(
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
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Plane"))
	{
		shared_ptr<Mesh> mesh;
		shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Plane.txt");
		importer->importMesh(mesh);
		shared_ptr<Object> object = make_shared<Object>(mesh);
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}
	
	if (ImGui::MenuItem("Floor"))
	{
		shared_ptr<Mesh> mesh;
		shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Floor.txt");
		importer->importMesh(mesh);
		shared_ptr<Object> object = make_shared<Object>(mesh);
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Sphere"))
	{
		shared_ptr<Object> sphere = make_shared<Sphere>(true);
		scene_objects.push_back(sphere);
		addObject(scene_objects, *sphere);
	}
	
	if (ImGui::MenuItem("Metaball"))
	{
		shared_ptr<Object> metaball = make_shared<Metaball>(1.0f);
		scene_objects.push_back(metaball);
		addObject(scene_objects, *metaball);
	}

	ImGui::SeparatorText("Landscape");
	if (ImGui::MenuItem("Terrain"))
	{
		shared_ptr<Object> terrain = make_shared<Terrain>(20.0f);
		scene_objects.push_back(terrain);
		addObject(scene_objects, *terrain);
	}
	
	ImGui::SeparatorText("Physics Simulator");
	if (ImGui::MenuItem("Cloth"))
	{
		shared_ptr<Object> cloth = make_shared<Cloth>();
		scene_objects.push_back(cloth);
		addObject(scene_objects, *cloth);
	}

	if (ImGui::MenuItem("Fluid"))
	{
		shared_ptr<SPHSystemCuda> fluid = make_shared<SPHSystemCuda>(64.0f, 32.0f, 64.0f);
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

ObjectPanel::ObjectPanel(string name) : 
	ImGuiPanel(name)
{}

ObjectPanel::~ObjectPanel()
{}

void ObjectPanel::render(
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	string name_clicked = "";	
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();

		if (scene_objects.size() == 0)
		{ 
			ImGui::End();
			return;
		}

		static vector<bool> selections(scene_objects.size(), false);
		if (selections.size() != scene_objects.size())
		{
			selections.resize(scene_objects.size());
			selections = vector<bool>(scene_objects.size(), false);
		}
		
		//for (int i = 0; i < scene_objects.size(); ++i)
		//{
		//	string name = scene_objects.at(i)->getIdName();
		//	if (name != name_clicked)
		//	{
		//		selections.at(i) = false;
		//	}
		//	else
		//	{
		//		selections.at(i) = true;
		//	}
		//}

		if (clicked_object == nullptr)
		{
			selections = vector<bool>(scene_objects.size(), false);
		}
		else
		{
			name_clicked = clicked_object->getIdName();
		}

		for (int i = 0; i < scene_objects.size(); ++i)
		{
			string name = scene_objects.at(i)->getIdName();
			
			if (ImGui::Selectable(name.c_str(), (selections.at(i) || name == name_clicked)))
			{
				if (!ImGui::GetIO().KeyCtrl)
				{
					selections = vector<bool>(scene_objects.size(), false);
					//memset(&selections, 0, sizeof(selections));
				}

				cout << "Scene " << scene_objects.size() << endl;
				cout << "selection: " << selections.size() << endl;

				bool select = selections.at(i);
				select ^= 1;
				selections.at(i) = select;
			}
		}

		for (int i = 0; i < selections.size(); ++i)
		{
			if (selections.at(i) == true)
			{
				scene_objects.at(i)->setIsClick(true);
				clicked_object = scene_objects.at(i);
				break;
			}
		}
	}
	ImGui::End();
}

PropertyPanel::PropertyPanel(string name) :
	ImGuiPanel(name),
	m_preview_fb(make_unique<FrameBuffer>()), m_preview_object(make_unique<Sphere>(false))
{
	m_preview_fb->createBuffers(256, 256);
}

PropertyPanel::~PropertyPanel()
{}

void PropertyPanel::render(
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
		//ImGui::Text(object_name);
		
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

PopupPanel::PopupPanel(string name) : ImGuiPanel(name) {}

PopupPanel::~PopupPanel() {}

void PopupPanel::popup(
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

				render(scene_objects, clicked_object);

				ImGui::EndPopup();
			}
			else
			{
				clicked_object->setIsPopup(false);
			}
		}
	}
}

void PopupPanel::render(vector<shared_ptr<Object>>& scene_objects,
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
