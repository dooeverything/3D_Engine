#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <list>

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
	void addObject(const string& mesh_path, const vector<string>& shader_path);
	void run();
	void render();

	void handleInput();
	void handleTransform(int axis);
	bool handlePicking(int w, int h, int index);
	
	void renderImGui();
	void renderDocking();
	void renderScene(int width, int height);
	void renderGizmo(int width, int height);
	
	void end();

private:
	unique_ptr<SDL_GL_Window> m_sdl_window;
	
	unique_ptr<Camera> m_camera;
	unique_ptr<FrameBuffer> m_framebuffer;
	
	vector<shared_ptr<Object>> m_objects;

	ImVec2 m_scene_min;
	ImVec2 m_scene_max;

	long long m_start_time;
	float m_ticks;
	
	bool m_is_running;
	bool m_is_mouse_clicked;
	bool m_pick;
	bool m_axis;
	bool m_click = false;
	
	int m_x;
	int m_y;
	int m_final_x;
	int m_final_y;
	int m_axis_selected = 1;

};

#endif