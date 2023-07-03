#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <list>

#include "SDL_GL_Window.h"
#include "Object.h"
#include "Camera.h"
#include "Light.h"
#include "ImGuiPanel.h"

using namespace std;

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool init();
	void run();
	void render();

	void handleInput();
	void moveObject(GameObject& go);

	void renderImGui();
	void renderScene(int width, int height);

	void end();

private:
	unique_ptr<SDL_GL_Window> m_sdl_window;
	vector<SDL_Event> m_frame_events;
	
	vector<shared_ptr<GameObject>> m_scene_objects;
	shared_ptr<GameObject> m_click_object;
	unique_ptr<Grid> m_grid;
	
	vector<shared_ptr<ImGuiPanel>> m_panels;
	
	unique_ptr<FrameBuffer> m_framebuffer;
	unique_ptr<ShadowMap> m_shadow_map;
	unique_ptr<Outline> m_outline;
	
	unique_ptr<Camera> m_camera;

	long long m_start_time;
	float m_ticks;
	
	bool m_is_running;
	bool m_is_mouse_down;
	bool m_mouse_in_panel;
	bool m_is_drag;
	bool m_is_click_gizmo;
	bool m_is_moving_gizmo;
};

#endif