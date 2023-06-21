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
	void addObject(const string& mesh_path, const vector<string> shader_path);
	void run();
	void render();

	void handleInput();
	void moveObject(GameObject& go);

	void renderImGui();
	void renderScene(int width, int height);
	
	void end();

private:
	unique_ptr<SDL_GL_Window> m_sdl_window;
	
	unique_ptr<Camera> m_camera;
	unique_ptr<FrameBuffer> m_framebuffer;
	
	vector<shared_ptr<GameObject>> m_scene_objects;
	unique_ptr<Grid> m_grid;

	long long m_start_time;
	float m_ticks;
	
	bool m_is_running;
	bool m_is_mouse_down;
	bool m_is_click_gizmo;
	bool m_is_drag;
};

#endif