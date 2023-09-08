#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <list>

#include "SDL_GL_Window.h"
#include "Object.h"
#include "MarchingCube.h"
#include "Camera.h"
#include "Light.h"
#include "ImGuiPanel.h"
#include "SPHSystem.h"

using namespace std;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void init();
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
	unique_ptr<Grid> m_grid;
	unique_ptr<FrameBuffer> m_framebuffer;

	unique_ptr<ShadowMap> m_shadow_map;
	unique_ptr<CubeMap> m_cubemap;
	unique_ptr<IrradianceMap> m_irradiancemap;
	unique_ptr<PrefilterMap> m_prefilter;
	unique_ptr<LUTMap> m_lut;
	unique_ptr<EnvironmentMap> m_environment;
	
	unique_ptr<Outline> m_outline;
	shared_ptr<GameObject> m_click_object;

	vector<SDL_Event> m_frame_events;
	vector<shared_ptr<GameObject>> m_scene_objects;
	vector<shared_ptr<Light>> m_lights;
	vector<shared_ptr<ImGuiPanel>> m_panels;
	
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