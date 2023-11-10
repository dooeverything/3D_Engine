#pragma once

#ifndef IMGUIPANEL_H
#define IMGUIPANEL_H

#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

#include "Object.h"
#include "Shader.h"
#include "Buffer.h"
#include "FileDialog.h"

class ImGuiPanel
{
public:
	ImGuiPanel(string name);
	~ImGuiPanel();
	virtual void render(
		vector<shared_ptr<GameObject>>& scene_objects, 
		shared_ptr<GameObject>& clicked_object) = 0;

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
	ImGuiMenuBar(string name);
	~ImGuiMenuBar();
	virtual void render(
		vector<shared_ptr<GameObject>>& scene_objects, 
		shared_ptr<GameObject>& clicked_object);

	virtual void addObject(
		vector<shared_ptr<GameObject>>& scene_objects, 
		GameObject& add_object);

private:
	unique_ptr<FileDialog> m_fd;
};

class ObjectPanel : public ImGuiPanel
{
public:
	ObjectPanel(string name);
	~ObjectPanel();
	virtual void render(
		vector<shared_ptr<GameObject>>& scene_objects, 
		shared_ptr<GameObject>& clicked_object);
};

class PropertyPanel : public ImGuiPanel
{
public:
	PropertyPanel(string name);
	~PropertyPanel();
	virtual void render(
		vector<shared_ptr<GameObject>>& scene_objects, 
		shared_ptr<GameObject>& clicked_object);

private:
	unique_ptr<FrameBuffer> m_preview_fb;
	unique_ptr<Sphere> m_preview_object;
	unique_ptr<FileDialog> m_fd;

	void renderPreview(Material& mat);
	void renderPreview(vector<shared_ptr<Texture>>& tex);

	void translatePanel(GameObject& clicked_object);
	void rotatePanel(GameObject& clicked_object);
	void scalePanel(GameObject& clicked_object);
};

class PopupPanel : public ImGuiPanel
{
public:
	PopupPanel(string name);
	~PopupPanel();

	void popup(vector<shared_ptr<GameObject>>& scene_objects,
		shared_ptr<GameObject>& clicked_object,
		bool& is_popup, bool& is_clicked_gizmo);

	void render(vector<shared_ptr<GameObject>>& scene_objects,
		shared_ptr<GameObject>& clicked_object);
};

#endif // !IMGUIPANEL_H
