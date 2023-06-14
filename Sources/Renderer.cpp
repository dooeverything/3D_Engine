#include "Renderer.h"

Renderer::Renderer() :
	m_sdl_window(make_unique<SDL_GL_Window>()), m_camera(unique_ptr<Camera>()),
	m_shader(make_unique<Shader>()), m_shader_grid(make_unique<Shader>()),
	m_framebuffer(make_unique<FrameBuffer>()),
	m_files({}), m_is_running(true), m_ticks(0), m_start_time(0),
	m_scene_min(0.0f, 0.0f), m_scene_max(0.0f, 0.0f), m_is_mouse_clicked(false), m_pick(false)
{}

Renderer::~Renderer() {}

bool Renderer::init()
{
	cout << "Initialize Renderer" << endl;

	int width = 1024;
	int height = 720;

	m_sdl_window->init(width, height, "Refractoring & ImGui");

	string file_path =  "Models/Link/Link.fbx";
	file_path = { "Models/Cube.txt" };

	loadFile(file_path);
	loadFile("Models/Arrow.fbx");
	loadFile("Models/Arrow.fbx");
	loadFile("Models/Arrow.fbx");
	loadFile("Models/Plane.txt");

	cout << "Number of object in scene: " << m_files.size() << endl;

	//m_shader->loadShaderFile("Shaders/Link.vert", "Shaders/Link.frag");
	m_shader->loadShaderFile("Shaders/Basic.vert", "Shaders/Basic.frag");
	m_shader_grid->loadShaderFile("Shaders/Grid.vert", "Shaders/Grid.frag");

	m_framebuffer->createBuffers(width, height);

	m_camera = make_unique<Camera>(glm::vec3(0.0f, 0.0f, 10.0f), -90.0f, 0.0f );

	m_start_time = SDL_GetTicks64();

	return true;
}

void Renderer::loadFile(const string& path)
{
	shared_ptr<FileLoader> file;
	string::size_type pos = path.find_last_of('.');
	if (pos != path.length())
	{
		string file_type = path.substr(pos + 1, path.length());
		cout << "Loaded file type : " << file_type << endl;
		if (file_type == "txt")
		{
			file = make_unique<TXTLoader>();
			file->loadMesh(path);
			m_files.push_back(file);
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
					//m_camera->processPicker(m_wsize.x, m_wsize.y, mouse_x, mouse_y);
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

bool Renderer::handlePicking(int w, int h, int index)
{
	//cout << "Handle picking: " << index << " " << m_files.at(index)->getPath() << endl;

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
	M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
	m_shader->load();
	m_shader->setPVM(P, V, M);
	m_files.at(0)->getMesh()->setPosition(M);
	//m_shader->setInt("animation", 0);
	m_shader->setVec3("light_pos", vec3(10.0f, 10.0f, 10.0f));
	m_shader->setVec3("object_color", vec3(1.0f, 0.5f, 0.31f));
	m_shader->setLight(*light);
	m_files.at(0)->getMesh()->draw();
	//m_files.front()->getMesh()->draw(*m_shader.get());
	
	// Handling picking object
	if (m_is_mouse_clicked)
	{
		m_is_mouse_clicked = false;
		m_pick = handlePicking(width, height, 0);
	}

	if (m_pick)
	{
		//cout << "Clicked?" << endl;
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
		m_shader_outline->setPVM(P, V, M);
		m_shader_outline->setFloat("outline", 0.05f);
		m_files.front()->getMesh()->draw();

		// #3 Enable modifying stencil buffer
		glStencilMask(0xFF);
		// Set stencil buffer with 0 --> clear stencil buffer
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		unique_ptr<Shader> m_shader_arrow = make_unique<Shader>();
		m_shader_arrow->loadShaderFile("Shaders/Arrow.vert", "Shaders/Arrow.frag");
		
		m_shader_arrow->load();
		M = mat4(1.0f);
		M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
		M = glm::translate(M, vec3(1.0f, 0.0f, 0.0f));
		M = glm::scale(M, vec3(0.5f));

		// Check if a gizmo is clicked
		glm::vec3 color_red = vec3(1.0f, 0.0f, 0.0f);
		m_files.at(1)->getMesh()->setPosition(M);
		if (handlePicking(width, height, 1))
		{
			color_red = vec3(0.5f, 0.0f, 0.0f);
			cout << "X-axis arrow is picked" << endl;
		}

		m_shader_arrow->setPVM(P, V, M);
		m_shader_arrow->setVec3("object_color", color_red);
		m_files.at(1)->getMesh()->draw(*m_shader_arrow.get());

		m_shader_arrow->load();
		M = mat4(1.0f);
		M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
		M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		M = glm::scale(M, vec3(0.5f));

		glm::vec3 color_green = vec3(0.0f, 1.0f, 0.0f);
		m_files.at(2)->getMesh()->setPosition(M);
		if (handlePicking(width, height, 2))
		{
			color_green = vec3(0.0f, 0.5f, 0.0f);
			cout << "Y-axis arrow is picked" << endl;
		}

		m_shader_arrow->setPVM(P, V, M);
		m_shader_arrow->setVec3("object_color", color_green);
		m_files.at(2)->getMesh()->draw(*m_shader_arrow.get());

		m_shader_arrow->load();
		M = mat4(1.0f);
		M = glm::translate(M, vec3(0.0f, 1.0f, 0.0f));
		M = glm::translate(M, vec3(0.0f, 0.0f, 1.0f));
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::scale(M, vec3(0.5f));

		glm::vec3 color_blue = vec3(0.0f, 0.0f, 1.0f);
		m_files.at(3)->getMesh()->setPosition(M);
		if (handlePicking(width, height, 3))
		{
			color_blue = vec3(0.0f, 0.0f, 0.5f);
			cout << "Z-axis arrow is picked" << endl;
		}

		m_shader_arrow->setPVM(P, V, M);
		m_shader_arrow->setVec3("object_color", color_blue);
		m_files.at(3)->getMesh()->draw(*m_shader_arrow.get());	
	}
	//Enable depth test
	glEnable(GL_DEPTH_TEST);
}

void Renderer::end()
{
	m_shader.release();
	/*m_model.release();*/
	m_sdl_window->unload();
}