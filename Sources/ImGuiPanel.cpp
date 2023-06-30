#include "ImGuiPanel.h"

ImGuiPanel::ImGuiPanel() : 
	m_scene_min(ImVec2(0.0, 0.0)), m_scene_max(ImVec2(0.0, 0.0))
{}

ImGuiPanel::~ImGuiPanel()
{}

void ImGuiPanel::render(vector<shared_ptr<GameObject>>& scene_objects)
{
	return;
}

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

	if (x < m_scene_min.x-10 || y < m_scene_min.y)
		return false;

	if (x > m_scene_max.x || y > m_scene_max.y)
		return false;

	return true;
}

ImGuiMenuBar::ImGuiMenuBar() : m_fd(nullptr)
{}

ImGuiMenuBar::~ImGuiMenuBar()
{}

void ImGuiMenuBar::render(vector<shared_ptr<GameObject>>& scene_objects)
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("GameObject"))
	{
		calculatePanelSize();
		if (ImGui::MenuItem("Create Empty", NULL)) {}

		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				addObject(scene_objects, "Models/Cube.txt");
			}
			if (ImGui::MenuItem("Plane"))
			{
				addObject(scene_objects, "Models/Plane.txt");
			}
			if (ImGui::MenuItem("Floor"))
			{
				addObject(scene_objects, "Models/Floor.txt");
			}
			if (ImGui::MenuItem("Sphere"))
			{
				shared_ptr<Sphere> sphere = make_shared<Sphere>();
				scene_objects.push_back(sphere);
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Import..."))
		{
			m_fd = make_unique<FileDialog>();
			string path = m_fd->OpenFile(".fbx");
			if (path != "")
			{
				cout << "Open: " << path << endl;
				addObject(scene_objects, path);
			}
		}

		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void ImGuiMenuBar::addObject(vector<shared_ptr<GameObject>>& scene_objects, const string& mesh_path)
{
	shared_ptr<GameObject> object = make_shared<GameObject>(mesh_path);
	scene_objects.push_back(object);
}

ObjectPanel::ObjectPanel()
{}

ObjectPanel::~ObjectPanel()
{}

void ObjectPanel::render(vector<shared_ptr<GameObject>>&scene_objects)
{
	bool click_object = false;
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();
		for (int i = 0; i < scene_objects.size(); ++i)
		{
			string name = scene_objects[i]->getName();
			const char* object_name = name.c_str();
			ImGui::Selectable(object_name, &click_object);
			if (click_object)
			{
				cout << name << " is clicked from panel" << endl;
				if (!scene_objects.at(i)->getIsClick())
					scene_objects.at(i)->setIsClick(true);
				else
					scene_objects.at(i)->setIsClick(false);
			}
			click_object = false;
		}
	}
	ImGui::End();
}

PropertyPanel::PropertyPanel()
{}

PropertyPanel::~PropertyPanel()
{}

void PropertyPanel::render(vector<shared_ptr<GameObject>>& scene_objects)
{
	ImGui::Begin("Property");
	{
		calculatePanelSize();
		for (int i = 0; i < scene_objects.size(); ++i)
		{
			if (scene_objects[i]->getIsClick())
			{
				string name = scene_objects[i]->getName();
				const char* object_name = name.c_str();
				ImGui::Text(object_name);
				bool expand_transform = ImGui::TreeNode("Transform");
				if (expand_transform)
				{
					static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
					ImVec2 cell_padding(0.0f, 2.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
					ImGui::BeginTable("Transform", 4);
					ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
					glm::vec3 pos = glm::vec3(0.0f);
					vector<string> transforms = { "Position", "Rotation", "Scale" };
					for (int row = 0; row < 3; ++row)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::AlignTextToFramePadding();
						ImGui::Text(transforms[row].c_str());
						ImGui::TableNextColumn();
						ImGui::Text("X");
						ImGui::SameLine();
						string id_x = "##x " + to_string(row);
						if (ImGui::DragFloat(id_x.c_str(), &(scene_objects[i]->getProperty(row)->x), 0.005f))
						{
							scene_objects[i]->setProperty(row, *scene_objects[i]->getProperty(row));
						}
						ImGui::TableNextColumn();
						ImGui::Text("Y");
						ImGui::SameLine();
						string id_y = "##y " + to_string(row);
						if (ImGui::DragFloat(id_y.c_str(), &(scene_objects[i]->getProperty(row)->y), 0.005f))
						{
							scene_objects[i]->setProperty(row, *scene_objects[i]->getProperty(row));
						}
						ImGui::TableNextColumn();
						ImGui::Text("Z");
						ImGui::SameLine();
						string id_z = "##z " + to_string(row);
						if (ImGui::DragFloat(id_z.c_str(), &(scene_objects[i]->getProperty(row)->z), 0.005f))
						{
							scene_objects[i]->setProperty(row, *scene_objects[i]->getProperty(row));
						}
						ImGui::TableNextColumn();
					}
					ImGui::EndTable();
					ImGui::PopStyleVar();
					ImGui::TreePop();
				}

				bool expand_material = ImGui::TreeNode("Material");
				if (expand_material)
				{
					static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
					ImGui::TreePop();
				}
			}
		}

	}
	ImGui::End();
}

