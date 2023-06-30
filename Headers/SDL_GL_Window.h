#pragma once

#include <iostream>

#include <SDL.h>
#include <GL/glew.h>

#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

using namespace std;

class SDL_GL_Window
{
private:
	SDL_Window* m_window;
	SDL_GLContext m_context;

	int m_width;
	int m_height;

	ImVec2 m_scene_min;
	ImVec2 m_scene_max;

public:
	int m_x;
	int m_y;

public:
	SDL_GL_Window();
	
	inline SDL_Window* getWindow() { return m_window; };
	inline int getWidth() { return m_width; };
	inline int getHeight() { return m_height; };
	inline ImVec2 getSceneMin() { return m_scene_min; };
	inline ImVec2 getSceneMax() { return m_scene_max; };

	void init(int width, int height, string title);
	void resizeWindow(int width, int height);
	void clearWindow();
	void swapWindow();
	void unload();

	inline void setScene(ImVec2 min, ImVec2 max)
	{
		m_scene_min = min;
		m_scene_max = max;
	}
};