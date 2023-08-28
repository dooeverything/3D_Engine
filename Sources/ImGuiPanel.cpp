#include "ImGuiPanel.h"
#include "Utils.h"

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
			if (ImGui::MenuItem("Metaball"))
			{
				shared_ptr<GameObject> metaball = make_shared<Metaball>(true);
				addObject(scene_objects, metaball);
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
		int last = int(add_object->getPath().find_last_of('/'));
		if (last == -1)
		{
			last = int(add_object->getPath().find_last_of('\\'));
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
	ImGui::Begin("Scene Objects");
	{
		calculatePanelSize();
		string name;
		static int selected_index = -1;
	
		if (clicked_object == nullptr)
		{
			selected_index = -1;
		}

		for (int i = 0; i < scene_objects.size(); ++i)
		{
			name = scene_objects.at(i)->getIdName();

			if (clicked_object != nullptr)
			{
				if (clicked_object->getIdName() == name )
				{
					selected_index = i;
				}
			}

			if (ImGui::Selectable(name.c_str(), selected_index == i, 0, ImVec2(64, 16)))
			{
				if (selected_index == i)
				{
					selected_index = -1;
					break;
				}
				selected_index = i;
			}
		}

		if (selected_index != -1)
		{
			if (clicked_object != nullptr)
			{
				string clicked_name = clicked_object->getName();
				int id = clicked_object->getId();
				if (name == clicked_name && id != 0)
				{
					cout << clicked_object->getId();
					cout << "HI " << clicked_name << endl;
					clicked_object = nullptr;
				}
				else
				{
					clicked_object = scene_objects.at(selected_index);
				}
			}
			else
			{
				clicked_object = scene_objects.at(selected_index);
			}
		}
		else
		{
			//cout << "No clicked object" << endl;
			clicked_object = nullptr;
		}
	}
	ImGui::End();
}

PropertyPanel::PropertyPanel() :
	m_preview_fb(make_unique<FrameBuffer>()), m_preview_object(make_unique<Sphere>(false))
{
	m_preview_fb->createBuffers(512, 512);
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
			//cout << "No clicked object" << endl;
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
	
		// Terrain Panel, if a gameobject is a terrain
		if (name == "Terrain")
		{
			Terrain* t = dynamic_cast<Terrain*>(clicked_object.get());
			bool expand_terrain = ImGui::TreeNode("Terrain");
			if (expand_terrain)
			{
				static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
				ImVec2 cell_padding(0.0f, 2.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
				ImGui::BeginTable("Terrain", 2);
				ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

				bool update = false;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Size");
				ImGui::TableNextColumn();
				string id_size = "##size";
				float size = t->getSize();
				if (ImGui::DragFloat(id_size.c_str(), &size, 1.0f))
				{
					t->setSize(size);
					update = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Grid Size");
				ImGui::TableNextColumn();
				string id_grid = "##grid";
				float grid_size = t->getGridSize();
				if (ImGui::DragFloat(id_grid.c_str(), &grid_size, 0.1f))
				{
					t->setGridSize(grid_size);
					update = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Threshold");
				ImGui::TableNextColumn();
				string id_threshold = "##threshold";
				float threshold = t->getThreshold();
				if (ImGui::DragFloat(id_threshold.c_str(), &threshold, 0.1f))
				{
					t->setThreshold(threshold);
					update = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Noise Scale");
				ImGui::TableNextColumn();
				string id_scale = "##noiseScale";
				int noise_scale = t->getNoiseScale();
				if (ImGui::DragInt(id_scale.c_str(), &noise_scale, 1))
				{
					t->setNoiseScale(noise_scale);
					update = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Octaves");
				ImGui::TableNextColumn();
				string id_octave = "##octaves";
				int octaves = t->getOctave();
				if (ImGui::DragInt(id_octave.c_str(), &octaves, 1))
				{
					t->setOctave(octaves);
					update = true;
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Frequency");
				ImGui::TableNextColumn();
				string id_freq = "##frequency";
				float frequency = t->getFrequency();
				if (ImGui::DragFloat(id_freq.c_str(), &frequency, 0.001f))
				{
					t->setFrequency(frequency);
					update = true;
				}

				if (update)
				{
					t->createWeights();
					t->updateVertex();
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				static int clicked = 0;
				if (ImGui::Button("Click to edit terrain"))
				{
					clicked++;
				}

				static int selected = -1;
				if (clicked & 1)
				{
					t->setIsEdit(true);
					ImGui::TableNextColumn();
					char buf1[32];
					sprintf_s(buf1, "Sculpting");
					if (ImGui::Selectable(buf1, selected == 0, 0, ImVec2(64, 16)))
						selected = 0;

					//ImGui::TableNextColumn();
					char buf2[32];
					sprintf_s(buf2, "Remove");
					if (ImGui::Selectable(buf2, selected == 1, 0, ImVec2(64, 16)))
						selected = 1;

				}
				else
				{
					selected = -1;
					t->setIsEdit(false);
				}

				if (selected == 0)
				{
					t->setStrength(1.0f);
				}
				else if(selected == 1)
				{
					t->setStrength(-1.0f);
				}

				ImGui::EndTable();

				//static int clicked = 0;

				ImGui::PopStyleVar();
				ImGui::TreePop();
			}
			else
			{
				t->setIsEdit(false);
			}
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
