#pragma once

#ifndef IMGUIBUTTON_H
#define IMGUIBUTTON_H

#include <iostream>
#include <vector>

#include "imgui-docking/imconfig.h"
#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_internal.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

using namespace std;

class Texture;

class ImGuiButton 
{
public:
	ImGuiButton(const string& path, const string& name);
	~ImGuiButton();

	inline bool getPress() { return m_press; };
	inline void setPress(bool press) { m_press = press; };
	bool draw();

private:
	unique_ptr<Texture> m_texture;
	string m_path;
	string m_name;
	bool m_press;
};

class ImGuiButtonTransform
{
public:
	ImGuiButtonTransform();
	~ImGuiButtonTransform();

	void draw(ImVec2 pos);
	int getTransformationType();

private:
	unique_ptr<ImGuiButton> m_button_cursor;
	unique_ptr<ImGuiButton> m_button_translation;
	unique_ptr<ImGuiButton> m_button_scale;
};

class ImGuiButtonPlay
{
public:
	ImGuiButtonPlay();
	~ImGuiButtonPlay();

	void draw(ImVec2 pos);
	bool getIsSimulate();

private:
	unique_ptr<ImGuiButton> m_button_play;
	unique_ptr<ImGuiButton> m_button_pause;
};

#endif // !IMGUIBUTTON_H