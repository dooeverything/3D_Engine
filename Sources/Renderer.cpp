#include "Renderer.h"

Renderer::Renderer() :
	m_sdl_window(make_unique<SDL_GL_Window>()), m_camera(unique_ptr<Camera>()),
	m_framebuffer(make_unique<FrameBuffer>()), 
	m_scene_objects({}), m_click_object(nullptr), m_shadow_map(nullptr),
	m_frame_events({}), m_is_running(true), m_ticks(0), m_start_time(0), m_is_mouse_down(false),
	m_is_click_gizmo(false), m_mouse_in_panel(false), m_is_moving_gizmo(false), m_is_drag(false)
{}

Renderer::~Renderer() {}

bool Renderer::init()
{
	cout << "Initialize Renderer" << endl;

	int width = 1400;
	int height = 800;
	m_sdl_window->init(width, height, "OpenGL Engine");
	m_grid = make_unique<Grid>();
	m_framebuffer->createBuffers(width, height);

	m_outline = make_unique<Outline>(width, height);

	m_camera = make_unique<Camera>(glm::vec3(0.0f, 0.5f, 20.0f), -90.0f, 0.0f );
	m_start_time = SDL_GetTicks64();

	glm::vec3 light_pos = { 4.0f, 4.0f, 4.0f };
	m_shadow_map = make_unique<ShadowMap>(light_pos);
	
	m_panels.push_back(make_shared<ImGuiMenuBar>());
	m_panels.push_back(make_shared<ObjectPanel>());
	m_panels.push_back(make_shared<PropertyPanel>());

	shared_ptr<GameObject> test_brdf = make_shared<Sphere>();
	string name = "Sphere-BRDF";
	test_brdf->setName(name);
	m_scene_objects.push_back(test_brdf);

	return true;
}

void Renderer::run()
{
	cout << "Render" << endl;
	while (m_is_running)
	{
		render();
	}
}

void Renderer::render()
{
	float current_frame = (float)SDL_GetTicks() * 0.001f;
	float last_frame = m_camera->getLastFrame();
	m_camera->setDeltaTime(current_frame - last_frame);
	m_camera->setLastFrame(current_frame);
	m_camera->processInput();

	handleInput();
	//m_sdl_window->clearWindow();

	// Draw a shadow map first to get shadows
	m_shadow_map->draw(m_scene_objects);
	renderImGui();
	m_sdl_window->swapWindow();
}

void Renderer::handleInput()
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	while (SDL_PollEvent(&event))
	{
		m_frame_events.push_back(event);

		if (!io.WantCaptureMouse)
		{
			switch (event.type)
			{
				case SDL_QUIT:
					m_is_running = false;
					break;
				
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						int width = event.window.data1;
						int height = event.window.data2;
						m_sdl_window->resizeWindow(width, height);
						cout << "window resize: " << width << " " << height << endl;
					}
					break;

				default:
					break;
			}
			m_frame_events.clear();
		}
		else if(!m_is_click_gizmo)
		{
			ImVec2 pos = io.MousePos;

			//cout << "Mouse Position " << pos.x << " " << pos.y << endl;
			for (auto& it : m_panels)
			{
				if (it->mouseInPanel(pos.x, pos.y) && !m_is_mouse_down)
				{
					//cout << "Mouse in panel" << endl;
					m_mouse_in_panel = true;
					break;
				}
				else
				{
					m_mouse_in_panel = false;
				}
			}

			ImVec2 min = m_sdl_window->getSceneMin();
			ImVec2 max = m_sdl_window->getSceneMax();

			int x, y, w, h;
			SDL_GetWindowPosition(m_sdl_window->getWindow(), &x, &y);
			SDL_GetWindowSize(m_sdl_window->getWindow(), &w, &h);
			if (pos.x < x || pos.y < y || pos.x > x + w || pos.y > y + h)
			{
				m_mouse_in_panel = true;
			}
			
			if (m_mouse_in_panel) continue;

			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type)
			{
				case SDL_MOUSEBUTTONUP:
					m_is_mouse_down = false;
					m_is_drag = false;
					m_camera->processMouseUp(event, m_sdl_window.get());
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (!m_mouse_in_panel && event.button.button == SDL_BUTTON_RIGHT)
					{
						//cout << "Mouse Down" << endl;
						m_is_drag = true;
					}
					else if (!m_mouse_in_panel && event.button.button == SDL_BUTTON_LEFT)
					{
						m_is_mouse_down = true;
					}
					m_camera->processMouseDown(event);
					m_frame_events.clear();
					return; // To avoid dragging when mouse is clicked

				case SDL_MOUSEMOTION:
					if (m_is_drag)
					{
						m_camera->processMouseDrag(event);
					}
					break;
			}
			m_frame_events.clear();
		}
	}
}

void Renderer::moveObject(GameObject& go)
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	float moveCellSize = 0.1f;
	for(auto& it : m_frame_events)
	{
		event = it;
		ImGui_ImplSDL2_ProcessEvent(&event);
		//cout << "Event: " << event.type << endl;
		switch (event.type)
		{
		case SDL_MOUSEBUTTONUP:
			m_is_moving_gizmo = false;
			go.getGizmo(go.m_move_axis)->setIsClick(false);
			break;

		case SDL_MOUSEBUTTONDOWN:
			cout << "move object along axis-" << go.m_move_axis << endl;
			go.getGizmo(go.m_move_axis)->setIsClick(true);
			m_is_moving_gizmo = true;
			break;

		case SDL_MOUSEMOTION:
			if (go.getGizmo(go.m_move_axis)->getIsClick())
			{
				int final_x;
				int final_y;
				SDL_GetRelativeMouseState(&final_x, &final_y);
				glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
				if (go.m_move_axis == 0)
				{
					if (m_camera->getForward().z <= 0)
					{
						if (final_x < -1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if(final_x > 1.0)
							pos[go.m_move_axis] = moveCellSize;
					}
					else
					{
						if(final_x > 1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if(final_x < -1.0)
							pos[go.m_move_axis] = moveCellSize;
					}
				}
				else if (go.m_move_axis == 1)
				{
					if (final_y < -1.0)
						pos[go.m_move_axis] = moveCellSize;
					else if(final_y > 1.0)
						pos[go.m_move_axis] = -moveCellSize;
				}
				else
				{
					if (m_camera->getForward().x >= 0.85f)
					{
						if (final_x < -1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if (final_x > 1.0)
							pos[go.m_move_axis] = moveCellSize;
					}
					else if (m_camera->getForward().x <= -0.9f)
					{
						if (final_x > 1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if (final_x < -1.0)
							pos[go.m_move_axis] = moveCellSize;
					}
					else if (m_camera->getForward().z < 0)
					{
						if (final_y < -1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if (final_y > 1.0)
							pos[go.m_move_axis] = moveCellSize;					
					}
					else
					{
						if (final_y > 1.0)
							pos[go.m_move_axis] = -moveCellSize;
						else if (final_y < -1.0)
							pos[go.m_move_axis] = moveCellSize;
					}
				}
				glm::vec3 curr_pos = *(go.getProperty(0));
				curr_pos += pos;
				go.setProperty(0, curr_pos);
				break;
			}
		}
	}

	m_frame_events.clear();
}

void Renderer::renderImGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	bool demo = true;
	ImGui::ShowDemoWindow(&demo);

	for (int i = 0; i < m_scene_objects.size(); ++i)
	{
		m_scene_objects.at(i)->setIsClick(false);
	}

	if (m_mouse_in_panel)
	{
		//cout << "Hello " << m_frame_events.size() << endl;
		SDL_Event event;
		ImGuiIO& io = ImGui::GetIO();
		float moveCellSize = 0.1f;
		for (auto& it : m_frame_events)
		{
			event = it;
			ImGui_ImplSDL2_ProcessEvent(&event);
		}
		m_frame_events.clear();
		
		m_mouse_in_panel = false;
	}

	// Draw panels
	for (auto& it : m_panels)
	{
		it->render(m_scene_objects, m_click_object);
	}

	glViewport(0, 0, 1400, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Slightly modified from https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
	ImGui::Begin("Scenes");
	{
		ImGui::BeginChild("SceneRender");
		{
			ImVec2 scene_min = ImGui::GetWindowContentRegionMin();
			ImVec2 scene_max = ImGui::GetWindowContentRegionMax();
			scene_min.x += ImGui::GetWindowPos().x;
			scene_min.y += ImGui::GetWindowPos().y;
			scene_max.x += ImGui::GetWindowPos().x;
			scene_max.y += ImGui::GetWindowPos().y;

			m_sdl_window->setScene(scene_min, scene_max);
			ImVec2 wsize = ImGui::GetWindowSize();

			if (m_click_object != nullptr)
			{
				float aspect = static_cast<float>(wsize.x) / static_cast<float>(wsize.y);
				glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
				glm::mat4 V = m_camera->camera2pixel();
				m_outline->setupBuffers(*m_click_object, P, V);
			}
			else
			{
				m_outline->clearOutlineFrame();
			}

			m_framebuffer->bind();
			m_sdl_window->clearWindow();
			renderScene((int)wsize.x, (int)wsize.y);
			m_framebuffer->unbind();
			ImGui::Image((ImTextureID)m_framebuffer->getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
	}
	ImGui::End();

	//ImGui::Begin("Debug");
	//{
	//	ImGui::BeginChild("DebugRenderer");
	//	{
	//		ImVec2 wsize = ImGui::GetWindowSize();
	//		ImGui::Image((ImTextureID)m_outline->getOutlineFrame(), wsize, ImVec2(0, 1), ImVec2(1, 0));
	//	}
	//	ImGui::EndChild();
	//}
	//ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}
}

void Renderer::renderScene(int width, int height)
{
	vector<shared_ptr<GameObject>> render_objects;
	for (auto& it : m_scene_objects)
	{
		render_objects.push_back(it);
	}

	glm::vec3 cam_pos = m_camera->getPos();

	// Setup light
	glm::vec3 dir = -(*m_shadow_map->getPosition());
	glm::vec3 amb = { 100.0f, 100.0f, 100.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> directional_light = make_unique<Light>(dir, amb, diff, spec);

	// Setup world to pixel coordinate transformation 
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	glm::mat4 V = m_camera->camera2pixel();

	// Calculate a ray to check whether object is clicked
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	int mouse_x = x - m_sdl_window->getSceneMin().x;
	int mouse_y = y - m_sdl_window->getSceneMin().y;
	m_camera->processPicker(width, height, mouse_x, mouse_y);
	glm::vec3 ray_dir = m_camera->getRay();
	glm::vec3 ray_pos = m_camera->getPos();

	std::sort(render_objects.begin(), render_objects.end(),
		[cam_pos](const shared_ptr<GameObject>& lhs, const shared_ptr<GameObject>& rhs)
		{
			float d1 = glm::length(cam_pos - *(lhs->getProperty(0)));
			float d2 = glm::length(cam_pos - *(rhs->getProperty(0)));
			return d1 < d2;
		});

	// Check whether any object is clicked 
	if (m_is_mouse_down && !m_is_click_gizmo && !m_mouse_in_panel && !m_is_drag)
	{
		for (int i = 0; i < render_objects.size(); ++i)
		{
			if (render_objects.at(i)->isClick(ray_dir, ray_pos))
			{
				cout << "Clicked object: " << render_objects.at(i)->getName() << endl;
				m_is_mouse_down = false;
				m_click_object = render_objects.at(i);
				break;
			}
			else
			{
				m_click_object = nullptr;
			}
		}
	}

	if (m_click_object != nullptr)
	{
		m_click_object->setIsClick(true);
	}

	// Check whether any gizmos is clicked
	for (auto& it : render_objects)
	{
		if (it->getIsClick())
		{
			if (m_is_moving_gizmo)
			{
				moveObject(*it);
				continue;
			}
			if (it->isGizmoClick(ray_dir, ray_pos) )
			{
				moveObject(*it);
				m_is_click_gizmo = true;
			}
			else
			{
				m_is_click_gizmo = false;
			}
			break;
		}
	}

	for (auto it = render_objects.rbegin(); it != render_objects.rend(); ++it)
	{
		if (!it->get()->getIsClick())
		{
			it->get()->draw(P, V, *directional_light, cam_pos, *m_shadow_map);
		}
	}

	if (m_click_object != nullptr)
	{
		glDisable(GL_DEPTH_TEST);
		m_outline->draw(*m_click_object, P, V);
		glEnable(GL_DEPTH_TEST);
		m_click_object->draw(P, V, *directional_light, cam_pos, *m_shadow_map);
	}

	// Draw Grid
	m_grid->draw(P, V, cam_pos);
	
	// Draw gizmos for clicked objects
	glDisable(GL_DEPTH_TEST);
	for (auto& it : render_objects)
	{
		if (it->getIsClick())
		{
			it->drawGizmos(P, V, cam_pos);
		}
	}
	glEnable(GL_DEPTH_TEST);

}

void Renderer::end()
{
	m_sdl_window->unload();
}