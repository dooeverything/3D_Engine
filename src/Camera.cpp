#include "Camera.h"

Camera::Camera(glm::vec3 pos, float yaw, float pitch) :
	m_picker(make_unique<Picker>()), 
	m_projectMatrix(0.0f), m_scene_projectMatrix(0.0f), m_viewMatrix(0.0f),
	m_pos(pos), m_forward(0.0f, 0.0f, -1.0f), m_up(0.0f, 1.0f, 0.0f),
	m_yaw(yaw), m_pitch(pitch),
	m_first_click(true), m_mouse_x(0), m_mouse_y(0),
	m_last_frame(0.0f), m_delta_time(0.0f),
	m_width(0.0f), m_height(0.0f), m_scene_width(0.0f), m_scene_height(0.0f)
{}

Camera::~Camera() {}

void Camera::processInput()
{
	const float speed = 4.5f * m_delta_time;
	const Uint8* state = SDL_GetKeyboardState(NULL);

	glm::vec3 right = normalize(cross(m_forward, m_up));

	if (state[SDL_SCANCODE_W])
		m_pos += speed * m_forward; // Move forward
	else if (state[SDL_SCANCODE_S])
		m_pos -= speed * m_forward; // Move Backward
	else if (state[SDL_SCANCODE_A])
		m_pos -= speed * right;
	else if (state[SDL_SCANCODE_D])
		m_pos += speed * right;
}

void Camera::processMouseDrag(SDL_Event event)
{
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	int x, y;
	SDL_GetRelativeMouseState(&x, &y);
	float sensitivity = 0.1f;
	float offset_x = static_cast<float>(x) * sensitivity;
	float offset_y = -static_cast<float>(y) * sensitivity;
		
	if (m_first_click)
	{
		offset_x = 0.0f;
		offset_y = 0.0f;
		m_first_click = false;
	}

	m_yaw += offset_x;
	m_pitch += offset_y;

	if (m_pitch > 89.0f)
	{
		m_pitch = 89.0f;
	}
	else if (m_pitch < -89.0f)
	{
		m_pitch = -89.0f;
	}
	//cout << "Yaw: " << m_yaw << " Pitch: " << m_pitch << endl;
	//cout << "Pos: " << m_pos.x << " " << m_pos.y << " " << m_pos.z << endl;
	updateCamera();
}

void Camera::processPicker(int x, int y)
{
	m_picker->calcualteMouseRay(m_scene_width, m_scene_height, x, y, m_viewMatrix);
}

void Camera::processMouseUp(SDL_Event event, SDL_GL_Window* window)
{	
	SDL_ShowCursor(SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_FALSE);
	
	int x, y;
	SDL_GetMouseState(&x, &y);
	if ( (abs(m_mouse_x -x) == 0 && abs(m_mouse_y - y) == 0) || m_first_click)
	{
		SDL_WarpMouseInWindow(window->getWindow(), x, y);
	}
	else
	{
		SDL_WarpMouseInWindow(window->getWindow(), m_mouse_x, m_mouse_y);
	}

	m_first_click = true;
}

void Camera::processMouseDown(SDL_Event event)
{
	if (event.button.button == SDL_BUTTON_RIGHT)
	{
		//cout << "Mouse Down" << endl;
		SDL_GetMouseState(&m_mouse_x, &m_mouse_y);
	}
}

void Camera::updateCamera()
{
	glm::vec3 forward;
	float yaw = glm::radians(m_yaw);
	float pitch = glm::radians(m_pitch);

	forward.x = cos(yaw) * cos(pitch);
	forward.y = sin(pitch);
	forward.z = sin(yaw) * cos(pitch);

	m_forward = normalize(forward);
}
#include <iomanip> 
void Camera::updateProjection()
{
	float aspect = m_width / m_height;
	m_projectMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

void Camera::updateSceneProjection()
{
	float aspect = m_scene_width / m_scene_height;
	m_scene_projectMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

void Camera::updateView()
{
	glm::vec3 right = normalize(cross(m_forward, m_up));
	glm::vec3 new_up = cross(right, m_forward);

	glm::mat4 R = { right.x, new_up.x, -m_forward.x, 0,
					right.y, new_up.y, -m_forward.y, 0,
			     	right.z, new_up.z, -m_forward.z, 0,
					0,		 0,	        0,	         1 };

	glm::mat4 t = glm::mat4(1.0f);
	t[3][0] = -m_pos.x;
	t[3][1] = -m_pos.y;
	t[3][2] = -m_pos.z;

	m_viewMatrix = R * t;
}
