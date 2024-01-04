#pragma once
#ifndef IMGUIPANEL_H
#define IMGUIPANEL_H

#include <iostream>
#include <vector>

#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

#include "FileDialog.h"

class Object;
class ObjectCollection;
class Sphere;
class Material;
class FrameBuffer;
class Texture;

using namespace std;

class ImGuiPanel
{
public:
	ImGuiPanel(string name);
	~ImGuiPanel();
	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object) = 0;

	virtual void calculatePanelSize();
	virtual bool mouseInPanel(int x, int y);

private:
	ImVec2 m_scene_min;
	ImVec2 m_scene_max;
	string m_name;
};

class ImGuiMenuBar : public ImGuiPanel
{
public:
	ImGuiMenuBar();
	~ImGuiMenuBar();
	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects, 
		shared_ptr<Object>& clicked_object) override;

	virtual void addObject(
		const vector<shared_ptr<Object>>& scene_objects, Object& add_object);

private:
	unique_ptr<FileDialog> m_fd;
};

class SceneHierarchyPanel : public ImGuiPanel
{
public:
	SceneHierarchyPanel();
	~SceneHierarchyPanel();
	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects, 
		shared_ptr<Object>& clicked_object) override;
};

class PropertyPanel : public ImGuiPanel
{
public:
	PropertyPanel();
	~PropertyPanel();
	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects, 
		shared_ptr<Object>& clicked_object) override;

private:
	unique_ptr<FrameBuffer> m_preview_fb;
	unique_ptr<Sphere> m_preview_object;
};

class PopupObject : public ImGuiPanel
{
public:
	PopupObject();
	~PopupObject();

	void popup(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object,
		bool& is_popup, bool& is_clicked_gizmo);

	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object) override;
};

class PopupSceneHierarchy : public ImGuiPanel
{
public:
	PopupSceneHierarchy();
	~PopupSceneHierarchy();

	void popup(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object);

	virtual void render(
		shared_ptr<ObjectCollection>& collection,
		vector<shared_ptr<Object>>& scene_objects,
		shared_ptr<Object>& clicked_object) override;
	
};

#endif // !IMGUIPANEL_H
