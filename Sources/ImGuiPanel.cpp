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

	m_scene_min.x += ImGui::GetWindowPos().x;
	m_scene_min.y += ImGui::GetWindowPos().y;
	m_scene_max.x += ImGui::GetWindowPos().x;
	m_scene_max.y += ImGui::GetWindowPos().y;
}

bool ImGuiPanel::mouseInPanel(int x, int y)
{
	if (m_scene_min.x == 0 && m_scene_max.y == 0)
		return false;

	if (x < m_scene_min.x-10 || y < m_scene_min.y-50)
		return false;

	if (x > m_scene_max.x || y > m_scene_max.y)
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
	if (ImGui::BeginMenu("GameObject"))
	{
		calculatePanelSize();
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
				shared_ptr<GameObject> sphere = make_shared<Sphere>();
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
	m_preview_fb(make_unique<FrameBuffer>()),
	m_preview_object(make_unique<Sphere>())
{
	vector<string> shader_path = { "Shaders/Preview.vert", "Shaders/Preview.frag" };
	m_preview_shader = make_unique<Shader>(shader_path);
	
	m_preview_fb->createBuffers(1024, 1024);
	m_preview_shader->processShader();
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
			renderPreview(*mat);
			//static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
			ImGui::Image((ImTextureID)m_preview_fb->getTextureID(), ImVec2(100.0, 100.0), ImVec2(0, 1), ImVec2(1, 0));
			ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoOptions;
			glm::vec3 color_mat = mat->getAmbient();
			ImVec4 color = ImVec4(mat->getAmbient().x, mat->getAmbient().y, mat->getAmbient().z, 1.0);
			ImGui::ColorEdit3("Select color", (float*)&color, misc_flags);
			mat->setAmbient(glm::vec3(color.x, color.y, color.z));
			bool load_texture = ImGui::Button("Apply texture");
			if (load_texture)
			{
				m_fd = make_unique<FileDialog>();
				string path = m_fd->OpenFile(".png");
				if (path != "")
				{
					cout << "Open: " << path << endl;
					mat->addTexture(path);
				}
			}
			ImGui::TreePop();
		}

		//for (int i = 0; i < scene_objects.size(); ++i)
		//{
		//	if (scene_objects[i]->getIsClick())
		//	{
		//	}
		//}

	}
	ImGui::End();
}

void PropertyPanel::renderPreview(Material& mat)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	m_preview_fb->bind();
	glViewport(0, 0, 1024, 1024);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_preview_shader->load();
	m_preview_shader->setPVM(P, V, M);
	m_preview_shader->setVec3("mat.ambient", mat.getAmbient());
	m_preview_shader->setVec3("mat.diffuse", mat.getDiffuse());
	m_preview_shader->setVec3("mat.specular", mat.getSpecular());
	m_preview_shader->setFloat("mat.shininess", mat.getShininess());
	if (mat.getTexture() != nullptr)
	{
		m_preview_shader->setInt("has_texture", 1);
		glActiveTexture(GL_TEXTURE0);
		mat.getTexture()->setActive();
	}
	else
	{
		m_preview_shader->setInt("has_texture", 0);
	}
	m_preview_object->getMesh()->draw();
	m_preview_fb->unbind();
}

