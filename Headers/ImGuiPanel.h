#pragma once

#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

#include "Object.h"
#include "FileDialog.h"

class ImGuiPanel
{
public:
	ImGuiPanel();
	~ImGuiPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects);
	virtual void calculatePanelSize();
	virtual bool mouseInPanel(int x, int y);
	ImVec2 m_scene_min;
	ImVec2 m_scene_max;

protected:
};

class ImGuiMenuBar : public ImGuiPanel
{
public:
	ImGuiMenuBar();
	~ImGuiMenuBar();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects);
	void addObject(vector<shared_ptr<GameObject>>& scene_objects, const string& mesh_path);

private:
	unique_ptr<FileDialog> m_fd;

};

class ObjectPanel : public ImGuiPanel
{
public:
	ObjectPanel();
	~ObjectPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects);
};

class PropertyPanel : public ImGuiPanel
{
public:
	PropertyPanel();
	~PropertyPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects);
};

class MaterialPanel : public PropertyPanel
{
public:
	MaterialPanel();
	~MaterialPanel();
	virtual void render(vector<GameObject>& scene_objects);
};