#include "Renderer.h"
#include "imgui_internal.h"

#include "MapManager.h"
#include "Object.h"
#include "ObjectManager.h"
#include "ObjectCollection.h"
#include "Outline.h"
#include "SoftBodySolver.h"
#include "ShaderManager.h"
#include "Quad.h"
#include "Gizmo.h"
#include "Grid.h"
#include "ImGuiManager.h"

unique_ptr<Quad> Quad::m_quad = nullptr;
unique_ptr<ImGuiManager> ImGuiManager::m_manager = nullptr;
unique_ptr<ObjectManager> ObjectManager::m_object_manager = nullptr;
unique_ptr<MapManager> MapManager::m_manager = nullptr;

Renderer::Renderer() :
	m_click_object(nullptr),
	m_frame_events({}), m_lights({}), m_gizmos({}),
	m_is_running(true), m_ticks(0), m_start_time(0), m_is_mouse_down(false),
	m_is_click_gizmo(false), m_mouse_in_panel(false), m_is_moving_gizmo(false), m_is_drag(false),
	m_is_popup(false)
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
	m_sdl_window->init(width, height, "Renderer");

	m_framebuffer_multi = make_unique<FrameBuffer>();
	m_framebuffer_multi->createBuffers(1024, 1024, true);

	m_scene = make_unique<FrameBuffer>();
	m_scene->createBuffers(1024, 1024);

	m_grid = make_unique<Grid>();
	m_outline = make_unique<Outline>(1024, 1024);

	m_camera = make_unique<Camera>(glm::vec3(0.0f, 7.5f, 27.0f), -90.0f, -11.0f);
	
	ShaderManager::createShader("Default", info::BASIC_SHADER);
	ShaderManager::createShader("Preview", info::PREVIEW_SHADER);

	cout << "********************Loading Gizmos********************" << endl;
	m_gizmos.resize(2);
	m_gizmos.at(0) = make_shared<Gizmo>("assets/models/ArrowTranslation.fbx", "translation", 0);
	m_gizmos.at(1) = make_shared<Gizmo>("assets/models/ArrowScale.fbx", "scale", 2);
	cout << "********************Finish loading gizmo********************\n" << endl;

	m_scene_collections = make_shared<ObjectCollection>(-1);

	// Setup lights
	glm::vec3 light_pos = { 1.0f, 1.0f, 1.0f };
	glm::vec3 dir = -light_pos;
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	m_lights.push_back(make_unique<Light>(dir, amb, diff, spec));
}

void Renderer::run()
{

	// PBR setup
	cout << "********************PBR Setup********************" << endl;
	MapManager::getManager()->setupPBRMaps();
	cout << "********************PBR Setup finish******************** \n" << endl;

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

	m_scene_collections->resetObjects();

	MapManager::getManager()->setupShadowMap();

	handleInput();

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
			ImGui_ImplSDL2_ProcessEvent(&event);
			ImVec2 pos = io.MousePos;

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
			ImVec2 scene_min = m_sdl_window->getSceneMin();
			ImVec2 scene_max = m_sdl_window->getSceneMax();
			
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (m_click_object)
			{
				if (m_click_object->getIsPopup())
				{
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						m_frame_events.clear();
						return;
					}
				}
			}

			m_is_popup = ImGuiManager::getImGuiManager()->isMouseInPanel(pos);

			// if mouse is out of scene, then do nothing
			if ( (pos.x < scene_min.x || pos.y < scene_min.y) && !m_is_drag)
			{
				m_frame_events.clear();
				return;
			}
			
			if ( (pos.x > scene_max.x || pos.y > scene_max.y) && !m_is_drag)
			{
				m_frame_events.clear();
				return;
			}			
	
			switch (event.type)
			{
				case SDL_MOUSEBUTTONUP:
					m_is_mouse_down = false;
					m_is_drag = false;
					m_camera->processMouseUp(event, m_sdl_window.get());
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						m_is_drag = true;				
						if (m_click_object)
						{
							m_click_object->setIsPopup(true);
						}
					}
					else if (event.button.button == SDL_BUTTON_LEFT)
					{
						m_is_mouse_down = true;
					}

					m_is_popup = false;
					m_camera->processMouseDown(event);
					m_frame_events.clear();
					return; // To avoid dragging when mouse is clicked

				case SDL_MOUSEMOTION:
					if (m_is_drag)
					{
						if (m_click_object)
						{
							if (m_click_object->getIsPopup())
								break;
						}
						m_camera->processMouseDrag(event);
					}
					break;
			}

			m_frame_events.clear();
		}
	}
}

void Renderer::moveObject(Object& go)
{
	SDL_Event event;
	ImGuiIO& io = ImGui::GetIO();
	for(auto& it : m_frame_events)
	{
		event = it;
		ImGui_ImplSDL2_ProcessEvent(&event);		
		switch (event.type)
		{
		case SDL_MOUSEBUTTONUP:
			m_is_mouse_down = false;
			m_is_moving_gizmo = false;
			m_gizmos.at(go.getTransformType())->setIsClick(false);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				m_gizmos.at(go.getTransformType())->setIsClick(true);
				m_is_moving_gizmo = true;
				break;
			}

		case SDL_MOUSEMOTION:
			if (m_gizmos.at(go.getTransformType())->getIsClick())
			{
				go.calcTransform(m_camera->getForward());
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

	ImGuiManager::getImGuiManager()->drawMainMenu(m_scene_collections, m_click_object);

	ImGuiManager::getImGuiManager()->drawButtons();

	ImGuiManager::getImGuiManager()->drawPanels(m_scene_collections, m_click_object);

	bool open = true;
	ImGui::Begin("Scenes", &open);
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

			// setup Depth map
			MapManager::getManager()->setupDepthMap(P, V);

			if (m_click_object != nullptr)
				m_outline->setupBuffers(*m_click_object, V, wsize.x, wsize.y);
			else
				m_outline->clearOutlineFrame();

			m_framebuffer_multi->bind();
			m_sdl_window->clearWindow();
			renderScene();

			m_framebuffer_multi->bindRead();
			m_scene->bindDraw();
			glBlitFramebuffer(0, 0, m_framebuffer_multi->getWidth(), m_framebuffer_multi->getHeight(),
				0, 0, m_scene->getWidth(), m_scene->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
			m_framebuffer_multi->unbind();

			ImGui::Image((ImTextureID)m_scene->getTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::EndChild();
	}
	ImGui::End();

	// ImGui docking
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
	glViewport(0, 0, m_framebuffer_multi->getWidth(), m_framebuffer_multi->getHeight());

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

	checkObjectClick(ray_dir, ray_pos);

	if (m_is_popup) 
		ImGuiManager::getImGuiManager()->popupHierarchyMenu(m_scene_collections, m_click_object);

	bool is_popup = false;
	ImGuiManager::getImGuiManager()->popupObjectMenu(m_scene_collections, m_click_object, is_popup, m_is_click_gizmo);

	// Check whether any gizmos is clicked
	checkGizmoClick(ray_dir, ray_pos, is_popup);

	ObjectManager::getObjectManager()->removeObject(m_scene_collections);

	ObjectManager::getObjectManager()->resetObjectIds();

	ObjectManager::getObjectManager()->drawObjects(P, V, ray_pos, *m_lights.at(0));

	MapManager::getManager()->drawCubeMap(P, V);

	// Draw Grid
	m_grid->draw(P, V, ray_pos);

	// Draw Outline
	m_outline->draw(*m_click_object);

	int type = ImGuiManager::getImGuiManager()->getTransformationType();
	m_gizmos.at(0)->checkDraw(type);
	m_gizmos.at(1)->checkDraw(type);

	// Draw gizmos for clicked objects
	if (m_click_object != nullptr)
	{
		m_gizmos.at(0)->draw(P, V, ray_pos);
		m_gizmos.at(1)->draw(P, V, ray_pos);
	}
}

void Renderer::checkObjectClick(const glm::vec3& ray_dir, const glm::vec3& ray_pos)
{
	// Check whether any object is clicked 
	if (m_is_mouse_down && !m_is_drag && !m_is_click_gizmo)
	{
		bool is_click = ObjectManager::getObjectManager()->checkObjectClick(m_click_object, ray_dir, ray_pos);
		if (!is_click)
		{
			if (m_click_object != nullptr)
			{
				if (m_click_object->getIsPopup() == false)
					m_click_object = nullptr;
			}
		}
	}
}

void Renderer::checkGizmoClick(const glm::vec3& ray_dir, const glm::vec3& ray_pos, bool is_popup)
{
	// Check whether any gizmos is clicked
	if (m_is_moving_gizmo)
	{
		m_gizmos.at(0)->computeBBox(m_click_object->getCenter(), ray_pos);
		m_gizmos.at(1)->computeBBox(m_click_object->getCenter(), ray_pos);
		moveObject(*m_click_object);
	}
	else if (m_click_object != nullptr && is_popup == false && !m_is_drag)
	{
		m_gizmos.at(0)->computeBBox(m_click_object->getCenter(), ray_pos);
		m_gizmos.at(1)->computeBBox(m_click_object->getCenter(), ray_pos);

		for (int i = 0; i < 2; ++i)
		{
			if (m_gizmos.at(i)->getIsDraw())
			{
				if (m_gizmos.at(i)->clickAxis(ray_dir, ray_pos))
				{
					m_is_click_gizmo = true;
					m_click_object->setMoveAxis(m_gizmos.at(i)->getIsAxisRayHit() - 1);
					m_click_object->setTransformType(i);
					moveObject(*m_click_object);
				}
				else
				{
					m_is_click_gizmo = false;
					m_click_object->setMoveAxis(-1);
				}
			}
		}
	}
}

void Renderer::end()
{
	m_sdl_window->unload();
}