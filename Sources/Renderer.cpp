#include "Renderer.h"

Renderer::Renderer() :
	m_sdl_window(make_unique<SDL_GL_Window>()), m_camera(unique_ptr<Camera>()),
	m_framebuffer(make_unique<FrameBuffer>()), m_scene_objects({}), 
	m_frame_events({}),
	m_is_running(true), m_ticks(0), m_start_time(0), m_is_mouse_down(false),
	m_is_click_gizmo(false), m_is_drag(false), m_is_moving_gizmo(false)
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
	m_camera = make_unique<Camera>(glm::vec3(0.0f, 0.5f, 20.0f), -90.0f, 0.0f );
	m_start_time = SDL_GetTicks64();

	return true;
}

void Renderer::addObject(const string& mesh_path)
{
	shared_ptr<GameObject> object = make_shared<GameObject>(mesh_path);
	m_scene_objects.push_back(object);
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

	m_sdl_window->clearWindow();
	renderImGui();
	m_sdl_window->swapWindow();
}

void Renderer::handleSimple()
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	while (SDL_PollEvent(&event))
	{
		//ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type)
		{
			case SDL_MOUSEBUTTONUP:
				m_is_mouse_down = false;
				m_is_drag = false;
				m_camera->processMouseUp(event, m_sdl_window.get());
				break;

			case SDL_MOUSEBUTTONDOWN:
				cout << "Mouse down" << endl;
				m_is_mouse_down = true;
				m_camera->processMouseDown(event);
				break;

			case SDL_MOUSEMOTION:
				if (m_is_mouse_down)
					m_is_drag = true;
				m_camera->processMouseDrag(event);
				break;
		}
	}
}

void Renderer::handleInput()
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
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
			int x, y;
			SDL_GetGlobalMouseState(&x, &y);
			if (ImGui::IsMouseDragging(0, 10.0f) && !m_is_mouse_down)
				continue;

			if (x < m_sdl_window->getSceneMin().x || 
				y < m_sdl_window->getSceneMin().y+30) 
				continue;
			
			if (x > m_sdl_window->getSceneMax().x || 
				y > m_sdl_window->getSceneMax().y) 
				continue;

			switch (event.type)
			{
				case SDL_MOUSEBUTTONUP:
					m_is_mouse_down = false;
					m_is_drag = false;
					m_camera->processMouseUp(event, m_sdl_window.get());
					break;

				case SDL_MOUSEBUTTONDOWN:
					m_is_mouse_down = true;
					m_camera->processMouseDown(event);
					break;

				case SDL_MOUSEMOTION:
					if(m_is_mouse_down)
						m_is_drag = true;
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
	float moveCellSize = 0.05f;
	for(auto& it : m_frame_events)
	{
		event = it;
		ImGui_ImplSDL2_ProcessEvent(&event);
		//cout << "Event: " << event.type << endl;
		switch (event.type)
		{
			case SDL_MOUSEBUTTONUP:
				cout << "UP" << endl;
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
							cout << m_camera->getForward() << endl;
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
					go.setPosition(pos);
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

	//bool demo = true;
	//ImGui::ShowDemoWindow(&demo);

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("GameObject"))
	{
		if (ImGui::MenuItem("Create Empty", NULL)) {}
					
		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				addObject("Models/Cube.txt");
			}

			if (ImGui::MenuItem("Plane"))
			{
				addObject("Models/Plane.txt");
			}
			if (ImGui::MenuItem("Sphere"))
			{
				shared_ptr<Sphere> sphere = make_shared<Sphere>();
				m_scene_objects.push_back(sphere);
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Import..."))
		{
			fd = make_unique<FileDialog>();
			string path =  fd->OpenFile(".fbx");
			if (path != "")
			{
				cout << "open: " << path << endl;
				addObject(path);
			}
		}

		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	bool click_object = false;
	ImGui::Begin("Scene Objects");
	{
		for (int i = 0; i < m_scene_objects.size(); ++i)
		{
			string name = m_scene_objects[i]->getName();
			const char* object_name = name.c_str();
			ImGui::Selectable(object_name, &click_object);
			if (click_object)
			{
				cout << name << " is clicked" << endl;
				if (!m_scene_objects.at(i)->getIsClick())
					m_scene_objects.at(i)->setIsClick(true);
				else
					m_scene_objects.at(i)->setIsClick(false);
			}
			click_object = false;
		}
	}
	ImGui::End();

	ImGui::Begin("Property");
	{
		for (int i = 0; i < m_scene_objects.size(); ++i)
		{ 
			if (m_scene_objects[i]->getIsClick())
			{
				string name = m_scene_objects[i]->getName();
				const char* object_name = name.c_str();
				ImGui::Text(object_name);
				bool expand_transform = ImGui::TreeNode("Transform");
				if (expand_transform)
				{
					static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
					ImVec2 cell_padding(0.0f, 2.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
					ImGui::BeginTable("Transform", 4);
					ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
					glm::vec3 pos = glm::vec3(0.0f);
					for (int row = 0; row < 1; ++row)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Position");
						ImGui::TableNextColumn();
						
						ImGui::Text("X");
						ImGui::SameLine();
						if (ImGui::DragFloat("##x", &(m_scene_objects[i]->getPosition()->x), 0.005f))
						{
							glm::mat4 t = glm::mat4(1.0f);
							t = glm::translate(t, *m_scene_objects[i]->getPosition());
							//cout << t << endl;
							m_scene_objects[i]->getMesh()->setTransform(t);
						}
						ImGui::TableNextColumn();
						
						ImGui::Text("Y");
						ImGui::SameLine();
						if (ImGui::DragFloat("##y", &(m_scene_objects[i]->getPosition()->y), 0.005f))
						{
							glm::mat4 t = glm::mat4(1.0f);
							t = glm::translate(t, *m_scene_objects[i]->getPosition());
							//cout << t << endl;
							m_scene_objects[i]->getMesh()->setTransform(t);
						}
						ImGui::TableNextColumn();
						ImGui::Text("Z");
						ImGui::SameLine();
						if (ImGui::DragFloat("##z", &(m_scene_objects[i]->getPosition()->z), 0.005f))
						{
							glm::mat4 t = glm::mat4(1.0f);
							t = glm::translate(t, *m_scene_objects[i]->getPosition());
							//cout << t << endl;
							m_scene_objects[i]->getMesh()->setTransform(t);
						}
						ImGui::TableNextColumn();
					}
					ImGui::EndTable();
					ImGui::PopStyleVar();
					ImGui::TreePop();
				}
			
				bool expand_material = ImGui::TreeNode("Material");
				if (expand_material)
				{
					static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
					ImGui::TreePop();
				}
			}
		}
		
	}
	ImGui::End();

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
			m_framebuffer->bind();
			renderScene((int)wsize.x, (int)wsize.y);
			m_framebuffer->unbind();
			ImGui::Image((ImTextureID)m_framebuffer->getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
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

void Renderer::renderFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

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
			m_framebuffer->bind();
			//m_sdl_window->clearWindow();
			//m_framebuffer->rescaleFrame((int)wsize.x, (int)wsize.y);
			//glViewport(0, 0, (GLsizei)wsize.x, (GLsizei)wsize.y);
			renderScene((int)wsize.x, (int)wsize.y);
			m_framebuffer->unbind();
			
			ImGui::Image((ImTextureID)m_framebuffer->getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
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

void Renderer::renderScene(int width, int height)
{
	glm::vec3 cam_pos = m_camera->getPos();

	// Setup light
	glm::vec3 dir = { -0.2f, -1.0f, -0.3f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 1.0f, 1.0f, 1.0f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);

	// Setup world to pixel coordinate transformation 
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	glm::mat4 V = m_camera->camera2pixel();
	glm::mat4 M = glm::mat4(1.0f);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Calculate a ray to check whether object is clicked
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	int mouse_x = x - m_sdl_window->getSceneMin().x;
	int mouse_y = y - m_sdl_window->getSceneMin().y;
	m_camera->processPicker(width, height, mouse_x, mouse_y);
	glm::vec3 ray_dir = m_camera->getRay();
	glm::vec3 ray_pos = m_camera->getPos();

	// Check whether object is clicked 
	if (m_is_mouse_down && !m_is_click_gizmo && !m_is_drag)
	{
		for (auto& it : m_scene_objects)
		{
			if (it->isClick(ray_dir, ray_pos))
			{
				cout << "Clicked object: " << it->getName() << endl;
				it->setIsClick(true);
			}
			else 
			{
				//cout << "missed object: " << it->getName() << endl;
				it->setIsClick(false);
			}
		}
	}

	// Check whether any gizmos is clicked
	for (auto& it : m_scene_objects)
	{
		if (it->getIsClick())
		{
			if (m_is_moving_gizmo)
			{
				moveObject(*it);
				continue;
			}

			if (it->isGizmoClick(ray_dir, ray_pos))
			{
				m_is_click_gizmo = true;
				moveObject(*it);
			}
			else
			{
				m_is_click_gizmo = false;
			}
		}
	}

	// Draw objects
	for (auto& it : m_scene_objects)
		it->draw(P, V, *light, cam_pos);

	m_grid->draw(P, V, cam_pos);
}

void Renderer::end()
{
	m_sdl_window->unload();
}