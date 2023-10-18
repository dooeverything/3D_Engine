#include "Renderer.h"

Renderer::Renderer() :
	m_shadow_map(nullptr), m_click_object(nullptr),
	m_frame_events({}), m_scene_objects({}), m_lights({}), m_panels({}), m_gizmos({}),
	m_is_running(true), m_ticks(0), m_start_time(0), m_is_mouse_down(false),
	m_is_click_gizmo(false), m_mouse_in_panel(false), m_is_moving_gizmo(false), m_is_drag(false)
{
	m_sdl_window = make_unique<SDL_GL_Window>();
	init();
}

Renderer::~Renderer() {}

void Renderer::init()
{
	cout << "Initialize Renderer" << endl;
	m_start_time = SDL_GetTicks64();

	int width = 1400;
	int height = 800;
	m_sdl_window->init(width, height, "OpenGL Engine");

	m_framebuffer = make_unique<FrameBuffer>();
	m_framebuffer->createBuffers(2048, 2048);

	m_grid = make_unique<Grid>();
	m_outline = make_unique<Outline>(2048, 2048);

	m_camera = make_unique<Camera>(glm::vec3(0.0f, 7.5f, 27.0f), -90.0f, -11.0f);
	
	glm::vec3 light_pos = { 1.0f, 1.0f, 1.0f };
	m_depth_map = make_unique<ShadowMap>(2048, 2048);
	m_shadow_map = make_unique<ShadowMap>(2048, 2048, light_pos, false);
	m_cubemap = make_unique<CubeMap>(4096, 4096);
	m_irradiancemap = make_unique<IrradianceMap>(32, 32);
	m_prefilter = make_unique<PrefilterMap>(256, 256);
	m_lut = make_unique<LUTMap>(512, 512);

	m_panels.push_back(make_shared<ImGuiMenuBar>());
	m_panels.push_back(make_shared<ObjectPanel>());
	m_panels.push_back(make_shared<PropertyPanel>());

	for (int axis = 0; axis < 3; ++axis)
	{
		shared_ptr<Gizmo> gizmo = make_shared<Gizmo>(axis);
		m_gizmos.push_back(gizmo);
	}

	//shared_ptr<GameObject> cloth = make_shared<Cloth>();
	//m_scene_objects.push_back(cloth);

	m_sph = make_shared<SPHSystem>(32.0f, 32.0f, 16.0f);
	m_scene_objects.push_back(m_sph);

	// Setup lights
	glm::vec3 dir = -light_pos; //{ -0.2f, -1.0f, -0.3f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	m_lights.push_back(make_unique<Light>(dir, amb, diff, spec));
}

void Renderer::run()
{
	cout << "Render" << endl;

	// Setup a cubemap
	m_cubemap->drawMap();
	
	// Setup an irradiance map
	m_irradiancemap->drawMap(*m_cubemap->getCubemapBuffer());

	// Setup an prefilter map
	m_prefilter->drawMap(*m_cubemap->getCubemapBuffer());

	// Setup a lut map
	m_lut->drawMap();

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
				if (it->mouseInPanel(int(pos.x), int(pos.y)) && !m_is_drag)
				{
					//cout << "Mouse in panel" << endl;
					m_mouse_in_panel = true;
					break;
				}
				else
				{
					//cout << "Mouse not in panel" << endl;
					m_mouse_in_panel = false;
				}
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
						m_is_drag = true;
					else if (!m_mouse_in_panel && event.button.button == SDL_BUTTON_LEFT)
						m_is_mouse_down = true;
					m_camera->processMouseDown(event);
					m_frame_events.clear();
					return; // To avoid dragging when mouse is clicked

				case SDL_MOUSEMOTION:
					if (m_is_drag)
						m_camera->processMouseDrag(event);
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
		//cout << "Event: " << event.type << " move along " << go.m_move_axis << " Click? " << m_gizmos[go.m_move_axis]->getIsClick() << endl;
		
		switch (event.type)
		{
		case SDL_MOUSEBUTTONUP:
			m_is_mouse_down = false;
			m_is_moving_gizmo = false;
			m_gizmos[go.m_move_axis]->setIsClick(false);
			break;

		case SDL_MOUSEBUTTONDOWN:
			m_gizmos[go.m_move_axis]->setIsClick(true);
			m_is_moving_gizmo = true;
			break;

		case SDL_MOUSEMOTION:
			//cout << "move " << go.getName() << " along axis : " << go.m_move_axis << endl;
			if (m_gizmos[go.m_move_axis]->getIsClick())
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
		if (m_scene_objects[i]->getName() == "Fluid")
		{
			continue;
		}

		m_scene_objects.at(i)->setIsClick(false);
		m_scene_objects.at(i)->resetRayHit();
	}

	// Draw panels
	for (auto& it : m_panels)
	{
		//cout << "Render ImGui" << endl;
		it->render(m_scene_objects, m_click_object);
	}

	if (m_mouse_in_panel)
	{
		SDL_Event event;
		ImGuiIO& io = ImGui::GetIO();
		//float moveCellSize = 0.1f;
		for (auto& it : m_frame_events)
		{
			event = it;
			ImGui_ImplSDL2_ProcessEvent(&event);
		}
		m_frame_events.clear();		
		//m_mouse_in_panel = false;
	}

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
			m_camera->setSceneWidth(wsize.x);
			m_camera->setSceneHeight(wsize.y);
			m_camera->updateSceneProjection();

			m_camera->setWidth(float(m_sdl_window->getWidth()));
			m_camera->setHeight(float(m_sdl_window->getHeight()));
			m_camera->updateProjection();
			m_camera->updateView();

			const glm::mat4& P = m_camera->getSP();
			const glm::mat4& V = m_camera->getV();

			// Get Depth map
			m_depth_map->setProj(P);
			m_depth_map->setView(V);
			m_depth_map->draw(m_scene_objects);

			if (m_sph != nullptr)
			{
				m_sph->setupFrame(V, *m_depth_map, *m_cubemap, *m_camera);
			}

			if (m_click_object != nullptr)
				m_outline->setupBuffers(*m_click_object, V, wsize.x, wsize.y);
			else
				m_outline->clearOutlineFrame();

			m_framebuffer->bind();
			m_sdl_window->clearWindow();
			renderScene();
			m_framebuffer->unbind();

			ImGui::Image((ImTextureID)m_framebuffer->getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
	}
	ImGui::End();

	ImGui::Begin("Debug");
	{
		ImGui::BeginChild("DebugRenderer");
		{
			ImVec2 wsize = ImGui::GetWindowSize();
			if (m_sph != nullptr)
			{
				//ImGui::Image((ImTextureID)m_depth_map->getBuffer().getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
				ImGui::Image((ImTextureID)m_sph->getNormalFB().getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

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

void Renderer::renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_framebuffer->getWidth(), m_framebuffer->getHeight());

	vector<shared_ptr<GameObject>> render_objects;
	for (auto& it : m_scene_objects)
	{
		render_objects.push_back(it);
	}

	glm::vec3 cam_pos = m_camera->getPos();

	// Setup world to pixel coordinate transformation 
	const glm::mat4& P = m_camera->getSP();
	const glm::mat4& V = m_camera->getV();

	// Calculate a ray to check whether object is clicked
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	int mouse_x = x - int(m_sdl_window->getSceneMin().x);
	int mouse_y = y - int(m_sdl_window->getSceneMin().y);
	m_camera->processPicker(mouse_x, mouse_y);
	glm::vec3 ray_dir = m_camera->getRay();
	glm::vec3 ray_pos = m_camera->getPos();

	// Check whether any object is clicked 
	if (m_is_mouse_down && !m_mouse_in_panel && !m_is_drag && !m_is_click_gizmo)
	{
		for (int i = 0; i < m_scene_objects.size(); ++i)
		{
			m_scene_objects[i]->isClick(ray_dir, ray_pos);
		}

		if (!m_scene_objects.empty())
		{
			std::sort(render_objects.begin(), render_objects.end(),
				[](const shared_ptr<GameObject>& lhs, const shared_ptr<GameObject>& rhs)
				{
					float d1 = lhs->getMesh()->getRayHitMin();
					float d2 = rhs->getMesh()->getRayHitMin();
					return d1 < d2;
				});
		
			if(render_objects.at(0)->getIsClick())
			{
				m_click_object = render_objects.at(0);
			}
			else
			{
				m_click_object = nullptr;
			}
		}
	}

	// Check whether any gizmos is clicked
	if (m_click_object != nullptr)
	{
		if (m_is_moving_gizmo)
		{
			moveObject(*m_click_object);
		}
		else
		{
			for (int axis = 0; axis < 3; ++axis)
			{
				if (m_gizmos.at(axis)->isClick(ray_dir, ray_pos))
				{

					m_click_object->m_move_axis = axis;
					moveObject(*m_click_object);
					m_is_click_gizmo = true;
					break;
				}
				else
				{
					m_click_object->m_move_axis = -1;
					m_is_click_gizmo = false;
				}
			}
		}
	}

	// Draw objects
	for (auto it = render_objects.rbegin(); it != render_objects.rend(); ++it)
	{
		if (it->get()->getName() == "Fluid") continue;

		if (it->get()->getName() == "Cloth")
		{
			if (it->get()->getSimulate())
			{
				it->get()->update();
			}
		}

		it->get()->draw(P, V, *m_lights.at(0), cam_pos, 
			*m_shadow_map, *m_irradiancemap, *m_prefilter, *m_lut);
	}

	if (m_sph != nullptr)
	{
		if (m_sph->getSimulate())
		{
			m_sph->update();
		}
		m_sph->draw();
	}

	// Draw background
	m_cubemap->draw(P, V);

	// Draw Grid
	m_grid->draw(P, V, cam_pos);

	// Draw Outline
	if (m_click_object != nullptr)
	{
		if (m_click_object->getName() != "Fluid")
		{
			glDisable(GL_DEPTH_TEST);
			m_outline->draw(*m_click_object);
			glEnable(GL_DEPTH_TEST);
		}
	}

	// Draw gizmos for clicked objects
	glDisable(GL_DEPTH_TEST);
	if (m_click_object != nullptr && m_click_object->getName() != "Fluid")
	{
		for (int axis = 0; axis < 3; ++axis)
		{
			m_gizmos[axis]->draw(*m_click_object, P, V, cam_pos);
		}
	}
	glEnable(GL_DEPTH_TEST);
}

void Renderer::end()
{
	m_sdl_window->unload();
}