#pragma once

#include <iostream>

#include <SDL.h>
#include <GL/glew.h>

#include "imgui-docking/imgui.h"
#include "imgui-docking/backends/imgui_impl_sdl2.h"
#include "imgui-docking/backends/imgui_impl_opengl3.h"


using namespace std;

class SDL_GL_Window
{
private:
	SDL_Window* m_window;
	SDL_GLContext m_context;

	int m_width;
	int m_height;

public:
	SDL_GL_Window();
	
	inline SDL_Window* getWindow() { return m_window; };
	inline int getWidth() { return m_width; };
	inline int getHeight() { return m_height; };

	void init(int width, int height, string title);
	void resizeWindow(int width, int height);
	void clearWindow();
	void swapWindow();
	void unload();
};