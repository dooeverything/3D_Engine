#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <list>

#include "SDL_GL_Window.h"
#include "FileDialog.h"
#include "Object.h"
#include "Camera.h"
#include "Light.h"

using namespace std;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool init();
	void addObject(const string& mesh_path);
	void run();
	void render();

	void handleInput();
	void handleSimple();
	void moveObject(GameObject& go);

	void renderImGui();
	void renderFrame();
	void renderScene(int width, int height);
	
	void end();

private:
	unique_ptr<SDL_GL_Window> m_sdl_window;
	vector<SDL_Event> m_frame_events;
	
	unique_ptr<Camera> m_camera;
	unique_ptr<FrameBuffer> m_framebuffer;
	
	vector<shared_ptr<GameObject>> m_scene_objects;
	unique_ptr<Grid> m_grid;

	unique_ptr<FileDialog> fd;

	long long m_start_time;
	float m_ticks;
	
	bool m_is_running;
	bool m_is_mouse_down;
	bool m_is_drag;
	bool m_is_click_gizmo;
	bool m_is_moving_gizmo;
};

#endif