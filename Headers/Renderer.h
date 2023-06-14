#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include "SDL_GL_Window.h"
#include "FileLoader.h"
#include "Camera.h"
#include "Light.h"

using namespace std;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool init();
	void loadFile(const string& path);
	void run();
	void render();

	void handleInput();
	bool handlePicking(int w, int h, int index);
	void renderScene(int width, int height);
	void renderImGui();
	void renderDocking();
	void end();

private:
	unique_ptr<SDL_GL_Window> m_sdl_window;
	
	unique_ptr<Shader> m_shader;
	unique_ptr<Shader> m_shader_grid;

	unique_ptr<Camera> m_camera;
	unique_ptr<FrameBuffer> m_framebuffer;
	
	vector<shared_ptr<FileLoader>> m_files;

	bool m_is_running;
	float m_ticks;
	long long m_start_time;
	
	ImVec2 m_scene_min;
	ImVec2 m_scene_max;
	bool m_is_mouse_clicked;
	bool m_pick;
};

#endif