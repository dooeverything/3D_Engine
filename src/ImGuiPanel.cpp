#include "ImguiPanel.h"
#include "Utils.h"
#include "MarchingCube.h"
#include "Cloth.h"
#include "SPHSystemCuda.h"

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
	vector<shared_ptr<GameObject>>& scene_objects, 
	shared_ptr<GameObject>& clicked_object)
{
	ImGui::SeparatorText("Primitives");
	if (ImGui::MenuItem("Cube"))
	{
		shared_ptr<GameObject> object = make_shared<GameObject>("assets/models/Cube.txt");
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Plane"))
	{
		shared_ptr<GameObject> object = make_shared<GameObject>("assets/models/Plane.txt");
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}
	
	if (ImGui::MenuItem("Floor"))
	{
		shared_ptr<GameObject> object = make_shared<GameObject>("assets/models/Floor.txt");
		scene_objects.push_back(object);
		addObject(scene_objects, *object);
	}

	if (ImGui::MenuItem("Sphere"))
	{
		shared_ptr<GameObject> sphere = make_shared<Sphere>(true);
		scene_objects.push_back(sphere);
		addObject(scene_objects, *sphere);
	}
	
	if (ImGui::MenuItem("Metaball"))
	{
		shared_ptr<GameObject> metaball = make_shared<Metaball>(1.0f);
		scene_objects.push_back(metaball);
		addObject(scene_objects, *metaball);
	}

	ImGui::SeparatorText("Terrain Generation");
	if (ImGui::MenuItem("Terrain"))
	{
		shared_ptr<GameObject> terrain = make_shared<Terrain>(16.0f);
		scene_objects.push_back(terrain);
		addObject(scene_objects, *terrain);
	}
	
	ImGui::SeparatorText("Physics Simulator");
	if (ImGui::MenuItem("Cloth"))
	{
		shared_ptr<GameObject> cloth = make_shared<Cloth>();
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
			cout << "Open: " << path << endl;
			shared_ptr<GameObject> fbx_object = make_shared<GameObject>(path);
			scene_objects.push_back(fbx_object);
			addObject(scene_objects, *fbx_object);
		}
	}
}

void ImGuiMenuBar::addObject(
	vector<shared_ptr<GameObject>>& scene_objects, 
	GameObject& add_object)
{
	string name;
	if (add_object.getPath() != "")
	{
		auto last = add_object.getPath().find_last_of('/');
		if (last == -1)
		{
			last = add_object.getPath().find_last_of('\\');
		}
		string temp = add_object.getPath().substr(last + 1, add_object.getPath().length());
		name = temp.substr(0, temp.find_last_of('.'));
	}
	else
	{
		name = add_object.getName();
	}

	int count = 0;
	for (auto& it : scene_objects)
	{
		if (it->getName() == name) count++;
	}

	if (count != 0)
	{
		add_object.setId(count);
	}
	add_object.setName(name);
}

ObjectPanel::ObjectPanel(string name) : 
	ImGuiPanel(name)
{}

ObjectPanel::~ObjectPanel()
{}

void ObjectPanel::render(
	vector<shared_ptr<GameObject>>& scene_objects, 
	shared_ptr<GameObject>& clicked_object)
{
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();

		int selected_index = -1;
		if (clicked_object == nullptr)
		{
			selected_index = -1;
		}

		for (int i = 0; i < scene_objects.size(); ++i)
		{
			string name = scene_objects.at(i)->getIdName();

			if (clicked_object != nullptr)
			{
				string name_clicked = clicked_object->getIdName();
				if (name_clicked == name)
				{
					selected_index = i;
				}
			}

			if (ImGui::Selectable(name.c_str(), selected_index == i))
			{
				if (selected_index == i)
				{
					// unselect, if the selected_index is clicked again
					selected_index = -1;
					break;
				}

				selected_index = i;
				break;
			}
		}

		if (selected_index != -1)
		{
			clicked_object = scene_objects.at(selected_index);
		}
		else
		{
			clicked_object = nullptr;
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
	vector<shared_ptr<GameObject>>& scene_objects, 
	shared_ptr<GameObject>& clicked_object)
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
		ImGui::Text(object_name);
		
		// Transform panel
		bool expand_transform = ImGui::TreeNode("Transform");
		if (expand_transform)
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
			ImVec2 cell_padding(0.0f, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
			ImGui::BeginTable("Transform", 4);
			ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
			
			translatePanel(*clicked_object);
			rotatePanel(*clicked_object);
			scalePanel(*clicked_object);

			ImGui::EndTable();
			ImGui::PopStyleVar();
			ImGui::TreePop();
		}

		// Material Panel
		bool expand_material = ImGui::TreeNode("Material");
		if (expand_material)
		{
			shared_ptr<Material> mat = clicked_object->getMesh()->getMaterial();
			if (clicked_object->getMesh()->getTexture().size() > 0)
			{
				//cout << "Clicked object " << clicked_object->getName() << " , " << clicked_object->getMesh()->getTexture().size() << endl;
				vector<shared_ptr<Texture>> tex = clicked_object->getMesh()->getTexture();
				renderPreview(tex);
				ImGui::Image((ImTextureID)m_preview_fb->getTextureID(), ImVec2(100.0, 100.0), ImVec2(0, 1), ImVec2(1, 0));
			}
			else
			{
				renderPreview(*mat);
				ImGui::Image((ImTextureID)m_preview_fb->getTextureID(), ImVec2(100.0, 100.0), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				if (mat->getTexture() == nullptr)
				{
					ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_DisplayRGB;
					misc_flags |= ImGuiColorEditFlags_NoSidePreview;

					glm::vec3 color_mat = mat->getBaseColor();
					ImVec4 color = ImVec4(color_mat.x, color_mat.y, color_mat.z, 1.0);
					ImGui::Text("Color");
					ImGui::ColorPicker3("##Color", (float*)&color, misc_flags);
					mat->setBaseColor(glm::vec3(color.x, color.y, color.z));
					
					//static float slider_f = 0.5f;
					
					ImGui::Dummy(ImVec2(0.0f, 5.0f));
					
					ImGuiSliderFlags flags = ImGuiSliderFlags_None;
					
					ImVec2 cell_padding(-1.0f, 2.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
					ImGui::BeginTable("PBR", 2);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();
					
					ImGui::Text("Metallic");
					ImGui::TableNextColumn();
					float metallic = mat->getMetallic();
					ImGui::SliderFloat("##Metallic", &metallic, 0.0f, 1.0f, "%.3f", flags);
					mat->setMetallic(metallic);

					//ImGui::Dummy(ImVec2(0.0f, 5.0f));
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Roughness");
					ImGui::TableNextColumn();
					float roughness = mat->getRoughness();
					ImGui::SliderFloat("##Roughness", &roughness, 0.0f, 1.0f, "%.3f", flags);
					mat->setRoughness(roughness);
					
					ImGui::EndTable();
					ImGui::PopStyleVar();
				}

				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				bool load_texture = ImGui::Button("Apply texture");
				if (load_texture)
				{
					m_fd = make_unique<FileDialog>();
					string path = m_fd->OpenFile(".jpg");
					if (path != "")
					{
						cout << "Open: " << path << endl;
						mat->addTexture(path);
					}
				}
			}
			ImGui::TreePop();
		}
		
		clicked_object->renderProperty();

		
		if (clicked_object->getSoftBodySolver())
		{
			clicked_object->getSoftBodySolver()->renderProperty();
		}
	}
	ImGui::End();
}

void PropertyPanel::renderPreview(Material& mat)
{
	m_preview_fb->bind();
		glViewport(0, 0, 256, 256);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_preview_object->drawPreview(mat);
	m_preview_fb->unbind();
}

void PropertyPanel::renderPreview(vector<shared_ptr<Texture>>& tex)
{
	m_preview_fb->bind();
		glViewport(0, 0, 256, 256);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_preview_object->drawPreview(tex);
	m_preview_fb->unbind();
}

void PropertyPanel::translatePanel(GameObject& clicked_object)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Position");
	ImGui::TableNextColumn();
	ImGui::Text("X");
	ImGui::SameLine();
	string id_x = "##x1";
	if (ImGui::DragFloat(id_x.c_str(), &(clicked_object.getProperty(0)->x), 0.005f))
	{
		clicked_object.setProperty(0, *clicked_object.getProperty(0));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	if (ImGui::DragFloat("##y1", &(clicked_object.getProperty(0)->y), 0.005f))
	{
		clicked_object.setProperty(0, *clicked_object.getProperty(0));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	if (ImGui::DragFloat("##z1", &(clicked_object.getProperty(0)->z), 0.005f))
	{
		clicked_object.setProperty(0, *clicked_object.getProperty(0));
	}
	ImGui::TableNextColumn();
}

void PropertyPanel::rotatePanel(GameObject& clicked_object)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Rotation");
	ImGui::TableNextColumn();
	ImGui::Text("X");
	ImGui::SameLine();
	string id_x = "##x2";
	if (ImGui::DragFloat(id_x.c_str(), &(clicked_object.getProperty(1)->x), 0.1f))
	{
		clicked_object.setProperty(1, *clicked_object.getProperty(1));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	if (ImGui::DragFloat("##y2", &(clicked_object.getProperty(1)->y), 0.1f))
	{
		clicked_object.setProperty(1, *clicked_object.getProperty(1));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	if (ImGui::DragFloat("##z2", &(clicked_object.getProperty(1)->z), 0.1f))
	{
		clicked_object.setProperty(1, *clicked_object.getProperty(1));
	}
	ImGui::TableNextColumn();
}

void PropertyPanel::scalePanel(GameObject& clicked_object)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Scale");
	ImGui::TableNextColumn();
	ImGui::Text("X");
	ImGui::SameLine();
	string id_x = "##x3";
	if (ImGui::DragFloat(id_x.c_str(), &(clicked_object.getProperty(2)->x), 0.005f))
	{
		clicked_object.setProperty(2, *clicked_object.getProperty(2));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	if (ImGui::DragFloat("##y3", &(clicked_object.getProperty(2)->y), 0.005f))
	{
		clicked_object.setProperty(2, *clicked_object.getProperty(2));
	}
	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	if (ImGui::DragFloat("##z3", &(clicked_object.getProperty(2)->z), 0.005f))
	{
		clicked_object.setProperty(2, *clicked_object.getProperty(2));
	}
	ImGui::TableNextColumn();
}

PopupPanel::PopupPanel(string name) : ImGuiPanel(name)
{
}

PopupPanel::~PopupPanel()
{
}

void PopupPanel::popup(
	vector<shared_ptr<GameObject>>& scene_objects, 
	shared_ptr<GameObject>& clicked_object,
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

void PopupPanel::render(vector<shared_ptr<GameObject>>& scene_objects,
						shared_ptr<GameObject>& clicked_object)
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

	if (clicked_object)
	{
		if (clicked_object->getSoftBodySolver() == nullptr)
		{
			if (ImGui::MenuItem("Add SoftbodySolver"))
			{
				cout << "Add softbodysolver" << endl;
				clicked_object->addSoftBodySolver();
			}	
		}
		else
		{
			ImGui::Text("Already Added SoftBodySolver");
		}
	}
}
