#pragma once

#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include <iostream>
#include <vector>

#include "ImGuiPanel.h"
#include "ImGuiButton.h"

using namespace std;

class Object;
class ObjectCollection;

class ImGuiManager
{
public:
	ImGuiManager(ImGuiManager const&) = delete;
	ImGuiManager& operator=(ImGuiManager const&) = delete;

	static ImGuiManager* getImGuiManager();

	void drawMainMenu(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object);
	
	void drawPanels(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object);

	void drawButtons();

	void popupObjectMenu(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object,
		bool& is_popup, bool& is_clicked_gizmo);
	
	void popupHierarchyMenu(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object);

	bool isMouseInPanel(ImVec2 pos);

	int getTransformationType();

private:
	vector<shared_ptr<ImGuiPanel>> m_panels;
	
	unique_ptr<ImGuiMenuBar> m_main_menu;

	unique_ptr<ImGuiButtonTransform> m_button1;
	unique_ptr<ImGuiButtonPlay> m_button2;

	unique_ptr<PopupObject> m_popup_object;
	unique_ptr<PopupSceneHierarchy> m_popup_scene;

	static unique_ptr<ImGuiManager> m_manager;
	ImGuiManager();
};

#endif // !IMGUIMANAGER_H
