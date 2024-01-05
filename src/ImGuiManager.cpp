#include "ImGuiManager.h"

ImGuiManager::ImGuiManager()
{
	m_panels.push_back(make_shared<SceneHierarchyPanel>());
	m_panels.push_back(make_shared<PropertyPanel>());

	m_main_menu = make_unique<ImGuiMenuBar>();

	m_button1 = make_unique<ImGuiButtonTransform>();
	m_button2 = make_unique<ImGuiButtonPlay>();

	m_popup_object = make_unique<PopupObject>();
	m_popup_scene = make_unique<PopupSceneHierarchy>();
}

ImGuiManager* ImGuiManager::getImGuiManager()
{
	if (m_manager == nullptr)
	{
		m_manager = unique_ptr<ImGuiManager>(new ImGuiManager());
	}

	return m_manager.get();
}

void ImGuiManager::drawMainMenu(
	shared_ptr<ObjectCollection>& collection, 
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	ImGui::BeginMainMenuBar();
	{
		if (ImGui::BeginMenu("Add"))
		{
			m_main_menu->render(collection, scene_objects, clicked_object);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void ImGuiManager::drawPanels(
	shared_ptr<ObjectCollection>& collection, 
	vector<shared_ptr<Object>>& scene_objects,
	shared_ptr<Object>& clicked_object)
{
	for (auto& it : m_panels)
		it->render(collection, scene_objects, clicked_object);
}

void ImGuiManager::drawButtons()
{
	ImGuiWindowClass window_class;
	window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize;
	ImGui::SetNextWindowClass(&window_class);
	ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
	{
		float pos_x = 10.0f;
		float pos_y = ImGui::GetWindowSize().y / 2.0f;
		//cout << pos_x << " " << endl;
		
		m_button1->draw({pos_x, pos_y-10.0f});

		m_button2->draw({ImGui::GetWindowSize().x / 2.0f, pos_y - 10.0f});
	}
	ImGui::End();
}

void ImGuiManager::popupObjectMenu(
	shared_ptr<ObjectCollection>& collection, 
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object, 
	bool& is_popup, bool& is_clicked_gizmo)
{
	m_popup_object->popup(collection, scene_objects, clicked_object, is_popup, is_clicked_gizmo);
}

void ImGuiManager::popupHierarchyMenu(
	shared_ptr<ObjectCollection>& collection, 
	vector<shared_ptr<Object>>& scene_objects, 
	shared_ptr<Object>& clicked_object)
{
	m_popup_scene->popup(collection, scene_objects, clicked_object);
}

bool ImGuiManager::isMouseInPanel(ImVec2 pos)
{
	return m_panels.at(0)->mouseInPanel(pos.x, pos.y);
}

int ImGuiManager::getTransformationType()
{
	return m_button1->getTransformationType();
}






