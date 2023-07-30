#include "ImGuiPanel.h"

ImGuiPanel::ImGuiPanel() : 
	m_scene_min(ImVec2(0.0, 0.0)), m_scene_max(ImVec2(0.0, 0.0))
{}

ImGuiPanel::~ImGuiPanel()
{}

void ImGuiPanel::render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object)
{
	return;
}

void ImGuiPanel::calculatePanelSize()
{
	m_scene_min = ImGui::GetWindowContentRegionMin();
	m_scene_max = ImGui::GetWindowContentRegionMax();

	//cout << "Menu min:  " << m_scene_min.x << " " << m_scene_min.y << endl;
	//cout << "Menu max: " << m_scene_max.x << " " << m_scene_max.y << endl;


	//cout << "After" << endl;

	m_scene_min.x += ImGui::GetWindowPos().x;
	m_scene_min.y += ImGui::GetWindowPos().y;
	m_scene_max.x += ImGui::GetWindowPos().x;
	m_scene_max.y += ImGui::GetWindowPos().y;

	//cout << "Menu min:  " << m_scene_min.x << " " << m_scene_min.y << endl;
	//cout << "Menu max: " << m_scene_max.x << " " << m_scene_max.y << endl;
}

bool ImGuiPanel::mouseInPanel(int x, int y)
{
	if (m_scene_min.x == 0 && m_scene_max.y == 0)
		return false;

	if (x < m_scene_min.x|| y < m_scene_min.y-20)
		return false;

	if (x > m_scene_max.x+10 || y > m_scene_max.y)
		return false;

	return true;
}

ImGuiMenuBar::ImGuiMenuBar() : m_fd(nullptr)
{}

ImGuiMenuBar::~ImGuiMenuBar()
{}

void ImGuiMenuBar::render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object)
{
	ImGui::BeginMainMenuBar();
	m_scene_min = ImVec2(1 + ImGui::GetWindowPos().x, 10 + ImGui::GetWindowPos().y);
	m_scene_max = ImVec2(1400+ ImGui::GetWindowPos().x, 30+ ImGui::GetWindowPos().y);
	//cout << m_scene_min.x << " " << m_scene_min.y << endl;
	//cout << "Win pos: " << ImGui::GetWindowPos().x << " " << ImGui::GetWindowPos().y << endl;
	if (ImGui::BeginMenu("GameObject"))
	{
		if (ImGui::MenuItem("Create Empty", NULL)) {}

		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				shared_ptr<GameObject> object = make_shared<GameObject>("Models/Cube.txt");
				addObject(scene_objects, object);
			}
			if (ImGui::MenuItem("Plane"))
			{
				shared_ptr<GameObject> object = make_shared<GameObject>("Models/Plane.txt");
				addObject(scene_objects, object);
			}
			if (ImGui::MenuItem("Floor"))
			{
				shared_ptr<GameObject> object = make_shared<GameObject>("Models/Floor.txt");
				addObject(scene_objects, object);
			}
			if (ImGui::MenuItem("Sphere"))
			{
				shared_ptr<GameObject> sphere = make_shared<Sphere>(true);
				addObject(scene_objects, sphere);
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
				shared_ptr<GameObject> fbx_object = make_shared<GameObject>(path);
				addObject(scene_objects, fbx_object);
			}
		}

		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void ImGuiMenuBar::addObject(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& add_object)
{
	string name;
	if (add_object->getPath() != "")
	{
		int last = add_object->getPath().find_last_of('/');
		if (last == -1)
		{
			last = add_object->getPath().find_last_of('\\');
		}
		string temp = add_object->getPath().substr(last + 1, add_object->getPath().length());
		name = temp.substr(0, temp.find_last_of('.'));
	}
	else
	{
		name = add_object->getName();
	}

	int count = 0;
	for (auto& it : scene_objects)
	{
		if (it->getName() == name) count++;
	}

	if (count != 0)
	{
		add_object->setId(count);
	}
	add_object->setName(name);
	scene_objects.push_back(add_object);
}

ObjectPanel::ObjectPanel()
{}

ObjectPanel::~ObjectPanel()
{}

void ObjectPanel::render(vector<shared_ptr<GameObject>>&scene_objects, shared_ptr<GameObject>& clicked_object)
{
	bool click_object = false;
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();
		for (int i = 0; i < scene_objects.size(); ++i)
		{
			string name = scene_objects.at(i)->getIdName();
			const char* object_name = name.c_str();
			ImGui::Selectable(object_name, &click_object);
			if (click_object)
			{
				if (clicked_object != nullptr)
				{
					string clicked_name = clicked_object->getIdName();
					cout << "Unclick " << clicked_name << " " << clicked_object->getName() << endl;

					if (object_name == clicked_name)
					{
						clicked_object = nullptr;
					}
					else
					{
						clicked_object = scene_objects.at(i);
					}
				}
				else
				{
					clicked_object = scene_objects.at(i);
				}
				cout << name << " is clicked from panel" << endl;
			}
			click_object = false;
		}
	}
	ImGui::End();
}

PropertyPanel::PropertyPanel() :
	m_preview_fb(make_unique<FrameBuffer>()), m_preview_object(make_unique<Sphere>(false))
{
	m_preview_fb->createBuffers(512, 512);

	//vector<string> shader_path = { "Shaders/BRDF.vert", "Shaders/BRDF.frag" };
	//m_preview_shader = make_unique<Shader>(shader_path);
	//
	//m_preview_shader->processShader();
}

PropertyPanel::~PropertyPanel()
{}

void PropertyPanel::render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object)
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
				if (ImGui::DragFloat(id_x.c_str(), &(clicked_object->getProperty(row)->x), 0.005f))
				{
					clicked_object->setProperty(row, *clicked_object->getProperty(row));
				}
				ImGui::TableNextColumn();
				ImGui::Text("Y");
				ImGui::SameLine();
				string id_y = "##y " + to_string(row);
				if (ImGui::DragFloat(id_y.c_str(), &(clicked_object->getProperty(row)->y), 0.005f))
				{
					clicked_object->setProperty(row, *clicked_object->getProperty(row));
				}
				ImGui::TableNextColumn();
				ImGui::Text("Z");
				ImGui::SameLine();
				string id_z = "##z " + to_string(row);
				if (ImGui::DragFloat(id_z.c_str(), &(clicked_object->getProperty(row)->z), 0.005f))
				{
					clicked_object->setProperty(row, *clicked_object->getProperty(row));
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
				m_preview_object->setIrradiance(clicked_object->getIrradiance());
				m_preview_object->setPrefiler(clicked_object->getPrefiler());
				m_preview_object->setLUT(clicked_object->getLUT());

				//shared_ptr<Material> mat = clicked_object->getMesh()->getMaterial();
				renderPreview(*mat);

				//static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
				ImGui::Image((ImTextureID)m_preview_fb->getTextureID(), ImVec2(100.0, 100.0), ImVec2(0, 1), ImVec2(1, 0));
			
				if (mat->getTexture() == nullptr)
				{
					ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoOptions;
					glm::vec3 color_mat = mat->getBaseColor();
					ImVec4 color = ImVec4(color_mat.x, color_mat.y, color_mat.z, 1.0);
					ImGui::ColorEdit3("Select color", (float*)&color, misc_flags);
					mat->setBaseColor(glm::vec3(color.x, color.y, color.z));

					static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
					float metallic = mat->getMetallic();
					//static float slider_f = 0.5f;
					ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f, "%.3f", flags);
					mat->setMetallic(metallic);

					float roughness = mat->getRoughness();
					ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.3f", flags);
					mat->setRoughness(roughness);
				}
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
	}
	ImGui::End();
}

void PropertyPanel::renderPreview(Material& mat)
{
	m_preview_fb->bind();
	glViewport(0, 0, 512, 512);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_preview_object->drawPreview(mat);
	m_preview_fb->unbind();
}

void PropertyPanel::renderPreview(vector<shared_ptr<Texture>>& tex)
{
	m_preview_fb->bind();
	glViewport(0, 0, 512, 512);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_preview_object->drawPreview(tex);
	m_preview_fb->unbind();
}
