#include <cassert>
#include "SDL_GL_Window.h"

SDL_GL_Window::SDL_GL_Window() :
	m_window(nullptr), m_context(0), 
	m_width(0), m_height(0), 
	m_scene_min(ImVec2(0.0f, 0.0f)), m_scene_max(ImVec2(0.0f, 0.0f)),
	m_x(0), m_y(0)
{}

void SDL_GL_Window::init(const int width, const int height, string title)
{
	m_width = width;
	m_height = height;

	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Set version of SDL 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Set a buffer with 8 bits for each channel
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	// Set double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Hardware Acceleration
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, window_flags);

	if (!m_window)
	{
		printf("Failed to load window: %s \n", SDL_GetError());
		assert(0);
	}

	m_context = SDL_GL_CreateContext(m_window);
	SDL_GL_MakeCurrent(m_window, m_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	cout << "Create imgui context" << endl;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(); 
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 15.0f);

	const char* glsl_version = "#version 450";

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW. \n");
		assert(0);
	}

	glGetError();

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// Enable stencil buffer
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// If both depth and stencil test pass, then use the result of the stencil test
	// Replace with the masked fragment value
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Enable blend buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	printf("%s \n", glGetString(GL_VERSION));
}

void SDL_GL_Window::resizeWindow(int width, int height)
{
	SDL_SetWindowSize(m_window, width, height);
	glViewport(0, 0, width, height);
}

void SDL_GL_Window::clearWindow()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SDL_GL_Window::swapWindow()
{
	SDL_GL_SwapWindow(m_window);
}

void SDL_GL_Window::unload()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	// Terminate and clear all allocated SDL resources
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}
