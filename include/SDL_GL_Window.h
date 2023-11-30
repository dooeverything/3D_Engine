#pragma once

#ifndef SDL_GL_WINDOW_H
#define SDL_GL_WINDOW_H

#include <iostream>
#include <GL/glew.h>
#include <SDL.h>

#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_impl_sdl2.h"
#include "imgui-docking/imgui_impl_opengl3.h"

using namespace std;

class SDL_GL_Window
{
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

private:
	void setupImGui();

	SDL_Window* m_window;
	SDL_GLContext m_context;
	ImVec2 m_scene_min;
	ImVec2 m_scene_max;

	int m_x;
	int m_y;
	int m_width;
	int m_height;
};

#endif // !SDL_GL_WINDOW_H