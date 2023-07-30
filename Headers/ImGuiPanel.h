#pragma once

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
	ImGuiPanel();
	~ImGuiPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object);
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
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object);
	virtual void addObject(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& add_object);

private:
	unique_ptr<FileDialog> m_fd;

};

class ObjectPanel : public ImGuiPanel
{
public:
	ObjectPanel();
	~ObjectPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object);
};

class PropertyPanel : public ImGuiPanel
{
public:
	PropertyPanel();
	~PropertyPanel();
	virtual void render(vector<shared_ptr<GameObject>>& scene_objects, shared_ptr<GameObject>& clicked_object);

private:
	unique_ptr<FrameBuffer> m_preview_fb;
	unique_ptr<Sphere> m_preview_object;
	//unique_ptr<Shader> m_preview_shader;

	void renderPreview(Material& mat);
	void renderPreview(vector<shared_ptr<Texture>>& tex);

	unique_ptr<FileDialog> m_fd;
};

//class MaterialPanel : public PropertyPanel
//{
//public:
//	MaterialPanel();
//	~MaterialPanel();
//	virtual void render(vector<GameObject>& scene_objects);
//};