#include "Renderer.h"

Renderer::Renderer() :
	m_sdl_window(make_unique<SDL_GL_Window>()), m_camera(unique_ptr<Camera>()),
	m_framebuffer(make_unique<FrameBuffer>()),
	m_objects({}), m_is_running(true), m_ticks(0), m_start_time(0),
	m_scene_min(0.0f, 0.0f), m_scene_max(0.0f, 0.0f), 
	m_is_mouse_clicked(false), m_pick(false), m_axis(false)
{}

Renderer::~Renderer() {}

bool Renderer::init()
{
	cout << "Initialize Renderer" << endl;

	int width = 1024;
	int height = 720;

	m_sdl_window->init(width, height, "Refractoring & ImGui");

	loadFile("Models/Cube.txt");
	loadFile("Models/Arrow.fbx");
	loadFile("Models/Arrow.fbx");
	loadFile("Models/Arrow.fbx");

	loadFile("Models/Plane.txt");

	cout << "Number of objects in scene: " << m_objects.size() << endl;

	//m_shader->loadShaderFile("Shaders/Link.vert", "Shaders/Link.frag");
	m_shader->loadShaderFile("Shaders/Basic.vert", "Shaders/Basic.frag");
	m_shader_grid->loadShaderFile("Shaders/Grid.vert", "Shaders/Grid.frag");

	m_framebuffer->createBuffers(width, height);

	m_camera = make_unique<Camera>(glm::vec3(0.0f, 5.0f, 20.0f), -90.0f, 0.0f );

	m_start_time = SDL_GetTicks64();

	return true;
}

void Renderer::addObject(const string& mesh_path, const vector<string>& shader_path)
{
	shared_ptr<Object> object;
	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "Loaded file type : " << file_type << endl;
		if (file_type == "txt")
		{
			object = make_unique<GameObject>(mesh_path, shader_path);
			m_objects.push_back(object);
		}
		else if (file_type == "fbx")
		{
			file = make_unique<FBXLoader>();
			file->loadMesh(path);
			m_files.push_back(file);
		}
	}
}

void Renderer::run()
{
	cout << "Run" << endl;

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

	if(!m_axis) 
		handleInput();

	m_sdl_window->clearWindow();
	renderDocking();
	m_sdl_window->swapWindow();
	
}

void Renderer::handleInput()
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (!io.WantCaptureMouse)
		{
			switch (event.type)
			{
				case SDL_QUIT:
					m_is_running = 0;
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
		}
		else
		{
			int x, y;
			SDL_GetGlobalMouseState(&x, &y);

			if (x < m_scene_min.x || y < m_scene_min.y+30) continue;
			if (x > m_scene_max.x || y > m_scene_max.y) continue;

			int mouse_x = x - m_scene_min.x;
			int mouse_y = y - m_scene_min.y;
			switch (event.type)
			{
				case SDL_MOUSEBUTTONUP:
					m_camera->processMouseUp(event, m_sdl_window.get());
					break;

				case SDL_MOUSEBUTTONDOWN:
					m_is_mouse_clicked = true;
					m_camera->processMouseDown(event);
					break;

				case SDL_MOUSEMOTION:
					m_camera->processMouseDrag(event);
					break;
			}
		}
	}
}

void Renderer::handleTransform(int axis)
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	float moveCellSize = 0.05;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_MOUSEBUTTONUP:
			cout << "Up" << endl;
			m_axis = false;
			m_click = false;
			break;

		case SDL_MOUSEBUTTONDOWN:
			cout << "Down" << endl;
			m_click = true;
			SDL_GetGlobalMouseState(&m_x, &m_y);
			cout << m_x << " " << m_y << endl;
			break;

		case SDL_MOUSEMOTION:
			if (m_click)
			{
				cout << "Motion at " << axis  << endl;
				SDL_GetGlobalMouseState(&m_final_x, &m_final_y);
				cout << m_final_x << " " << m_final_y << endl;

				glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
				if (axis-1 == 0)
				{
					if ((m_final_x - m_x) < 0)
						pos[axis-1] = -moveCellSize;
					else
						pos[axis-1] = moveCellSize;
				}
				else if (axis - 1 == 1)
				{
					if ((m_final_y - m_y) < 0)
						pos[axis - 1] = moveCellSize;
					else
						pos[axis - 1] = -moveCellSize;
				}
				else
				{
					if ((m_final_y - m_y) < 0)
						pos[axis-1] = -moveCellSize;
					else
						pos[axis-1] = moveCellSize;
				}


				cout << pos.x << " " << pos.y << " " << pos.z << endl;

				m_files.at(0)->getMesh()->setPosition(pos);
			}
			break;
		}
	}
}

bool Renderer::handlePicking(int w, int h, int index)
{
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	int mouse_x = x - m_scene_min.x;
	int mouse_y = y - m_scene_min.y;
	m_camera->processPicker(w, h, mouse_x, mouse_y);

	glm::vec3 ray_dir = m_camera->getRay();
	glm::vec3 ray_pos = m_camera->getPos();

	return (m_files.at(index)->getMesh()->intersect(ray_dir, ray_pos));
}

void Renderer::renderImGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;

	//if (m_show_demo)
	//	ImGui::ShowDemoWindow(&m_show_demo);
	
	//ImGui::Begin("Setting");                          
	//{
	//	ImGui::Checkbox("Good", &m_show_demo);
	//	ImGui::End();
	//}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window,backup_current_context);
	}
}

void Renderer::renderDocking()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	bool show_demo = false;

	ImGui::Begin("Settings");
	{
		ImGui::Checkbox("Demo window", &show_demo);
	}
	ImGui::End();

	// Slightly modified from https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
	ImGui::Begin("Scenes");
	{
		ImGui::BeginChild("SceneRender");
		{
			m_scene_min = ImGui::GetWindowContentRegionMin();
			m_scene_max = ImGui::GetWindowContentRegionMax();
			
			m_scene_min.x += ImGui::GetWindowPos().x;
			m_scene_min.y += ImGui::GetWindowPos().y;
			m_scene_max.x += ImGui::GetWindowPos().x;
			m_scene_max.y += ImGui::GetWindowPos().y;

			ImVec2 wsize = ImGui::GetWindowSize();

			m_framebuffer->bind();
			m_sdl_window->clearWindow();
			m_framebuffer->rescaleFrame((int)wsize.x, (int)wsize.y);
			glViewport(0,0,(GLsizei)wsize.x, (GLsizei)wsize.y);
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
	// Setup light
	vec3 dir = { -0.2f, -1.0f, -0.3f };
	vec3 amb = { 1.0f, 1.0f, 1.0f };
	vec3 diff = { 1.0f, 1.0f, 1.0f };
	vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);

	// Setup world to pixel coordinate transformation 
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	mat4 V = m_camera->camera2pixel();
	mat4 M = mat4(1.0f);
	
	//M = mat4(1.0f);
	m_shader_grid->load();
	m_shader_grid->setPVM(P, V, M);
	m_shader_grid->setVec3("cam_pos", m_camera->getPos());
	m_files.at(4)->getMesh()->draw(*m_shader_grid.get());

	glClear(GL_DEPTH_BUFFER_BIT);
	// #1 Draw a original model
	// glStencilFunc -> stencil test, it always pass the stencil test
	// Set stencil buffer with 1 / 1 & (operation)0xFF -> 1
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	// Enable modifying stencil buffer
	glStencilMask(0xFF);

	M = mat4(1.0f);
	glm::vec3 pos = m_files.at(0)->getMesh()->getPosition();
	M = glm::translate(M, pos);
	m_shader->load();
	m_shader->setPVM(P, V, M);
	m_files.at(0)->getMesh()->setTransform(M);
	m_shader->setVec3("light_pos", vec3(10.0f, 10.0f, 10.0f));
	m_shader->setVec3("object_color", vec3(1.0f, 0.5f, 0.31f));
	m_shader->setLight(*light);
	m_files.at(0)->getMesh()->draw();
	
	// Handling picking object
	if (m_is_mouse_clicked)
	{
		m_is_mouse_clicked = false;
		m_pick = handlePicking(width, height, 0);
	}

	if (m_pick)
	{
		renderGizmo(width, height);
	}

	//Enable depth test
	glEnable(GL_DEPTH_TEST);
}

void Renderer::renderGizmo(int width, int height)
{
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	mat4 V = m_camera->camera2pixel();
	mat4 M = mat4(1.0f);

	unique_ptr<Shader> m_shader_outline = make_unique<Shader>();
	m_shader_outline->loadShaderFile("Shaders/Outline.vert", "Shaders/Outline.frag");

	// #2 Draw a larger model with white color
	// The pixel that is NOT equal to 1  will be passed
	// --> It will only draw a object where the stencil buffer is equal to 0
	// --> hence, it will only draw border of a model
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	// Disable modifying stencil buffer
	// Stencil buffer will not be updated when drawing larger model
	glStencilMask(0x00);
	// Disable depth test
	glDisable(GL_DEPTH_TEST);

	m_shader_outline->load();
	M = m_files.at(0)->getMesh()->getTransform();
	m_shader_outline->setPVM(P, V, M);
	m_shader_outline->setFloat("outline", 0.025f);
	m_files.front()->getMesh()->draw();

	// #3 Enable modifying stencil buffer
	glStencilMask(0xFF);
	// Set stencil buffer with 0 --> clear stencil buffer
	glStencilFunc(GL_ALWAYS, 0, 0xFF);

	unique_ptr<Shader> m_shader_arrow = make_unique<Shader>();
	m_shader_arrow->loadShaderFile("Shaders/Arrow.vert", "Shaders/Arrow.frag");
	
	if (!m_click)
	{
		for (int i=1; i < 4; i++)
		{
			m_axis = handlePicking(width, height, i);
			if (m_axis)
			{
				cout << "Pick: " << i << endl;
				m_axis_selected = i;
				break;
			}
		}
	}

	if (m_axis)
	{
		cout << "Axis : " << m_axis_selected << endl;
		handleTransform(m_axis_selected);
		if (m_click)
		{
			color[m_axis_selected -1] = 0.5f;
			cout << "Color: " << color.r << " " << color.g << " " << color.b << endl;
		}
	}

	// Draw x-axis(red) gizmo
	m_shader_arrow->load();
	M = mat4(1.0f);
	M = m_files.at(0)->getMesh()->getTransform();
	M = glm::translate(M, vec3(1.0f, 0.0f, 0.0f));
	M = glm::scale(M, vec3(0.5f));
	m_files.at(1)->getMesh()->setTransform(M);
	m_shader_arrow->setPVM(P, V, M);
	m_shader_arrow->setVec3("object_color", glm::vec3(color.r, 0.0f, 0.0f));
	m_files.at(1)->getMesh()->draw(*m_shader_arrow.get());

	// Draw y-axis(green) gizmo
	m_shader_arrow->load();
	M = mat4(1.0f);
	M = m_files.at(0)->getMesh()->getTransform();
	M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::scale(M, vec3(0.5f));
	m_files.at(2)->getMesh()->setTransform(M);
	m_shader_arrow->setPVM(P, V, M);
	m_shader_arrow->setVec3("object_color", glm::vec3(0.0f, color.g, 0.0f));
	m_files.at(2)->getMesh()->draw(*m_shader_arrow.get());

	// Draw a z-axis(blue) gizmo
	m_shader_arrow->load();
	M = mat4(1.0f);
	M = m_files.at(0)->getMesh()->getTransform();
	M = glm::translate(M, vec3(0.0f, 0.0f, 1.0f));
	M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::scale(M, vec3(0.5f));
	m_files.at(3)->getMesh()->setTransform(M);
	m_shader_arrow->setPVM(P, V, M);
	m_shader_arrow->setVec3("object_color", glm::vec3(0.0f, 0.0, color.b));
	m_files.at(3)->getMesh()->draw(*m_shader_arrow.get());
}

void Renderer::end()
{
	m_shader.release();
	/*m_model.release();*/
	m_sdl_window->unload();
}