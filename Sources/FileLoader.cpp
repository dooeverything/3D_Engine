#include "FileLoader.h"

Object::Object() :
	m_mesh(nullptr), m_shader(nullptr), 
	m_click(false), m_name("")
{}

Object::Object(const string& mesh_path, const vector<string>& shader) :
	m_click(false), m_name("")
{
	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "GameObject type : " << file_type << endl;
		if (file_type == "txt")
		{
			m_mesh = make_shared<Mesh>(mesh_path);
		}
		else if (file_type == "fbx")
		{
			m_mesh = make_shared<FBXMesh>(mesh_path);
		}
	}

	m_shader = make_shared<Shader>(shader);

	string temp = mesh_path.substr(mesh_path.find_last_of('/')+1, mesh_path.length());
	
	int length = temp.length() - temp.find_last_of('.');
	m_name = temp.substr(0, length);
}

Object::~Object()
{}

bool Object::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	return m_mesh->intersect(ray_dir, ray_pos);
}

Gizmo::Gizmo(GameObject& root, int axis) : 
	m_root(root), m_axis(axis)
{
	m_mesh = make_shared<FBXMesh>("Models/Arrow.fbx");
	m_mesh->processMesh();

	vector<string> shader_paths = {"Shaders/Arrow.vert", "Shaders/Arrow.frag"};
	m_shader = make_shared<Shader>(shader_paths);
	m_shader->processShader();

	cout << "Gizmos loaded succesfully..." << endl;
	cout << endl;
}

Gizmo::~Gizmo()
{}

void Gizmo::draw(glm::mat4& P, glm::mat4& V, glm::mat4& M)
{
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	//glm::mat4 M = glm::translate(glm::mat4(1.0f), m_root.getMesh()->getCenter());
	M = m_root.getMesh()->getTransform() * M;
	
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	pos[m_axis] = 1.8f;
	M = glm::translate(M, pos);

	if (m_axis == 1)
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	else if(m_axis == 2)
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	m_mesh->setTransform(M);

	if (m_click)
	{
		color[m_axis] = 0.5f;
	}
	else
	{
		color[m_axis] = 1.0f;
	}
	
	m_shader->load();
	m_shader->setVec3("object_color", color);
	m_mesh->draw(P, V, *m_shader);
}

//void Gizmo::isPicking(glm::vec3 ray_dir, glm::vec3 ray_pos)
//{
//	int axis = 0;
//	for (; axis < 3; ++axis)
//	{
//		if (m_axis[axis]->intersect(ray_dir, ray_pos))
//			break;
//	}
//	m_mehs
//	m_clicked_axis = axis;
//}

Grid::Grid()
{
	m_mesh = make_shared<Mesh>("Models/Plane.txt");
	vector<string> shader_path = { "Shaders/Grid.vert", "Shaders/Grid.frag" };
	m_shader = make_shared<Shader>(shader_path);

	m_mesh->processMesh();
	m_shader->processShader();

	cout << "Grid finish loading..." << endl;
	cout << endl;
}

void Grid::draw(glm::mat4& P, glm::mat4& V, glm::vec3 cam_pos)
{
	m_shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	m_shader->setPVM(P, V, M);
	m_shader->setVec3("cam_pos", cam_pos);
	m_mesh->draw();
}

//GameObject::GameObject() :
//	Object(), m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f))
//{
//	m_mesh->processMesh();
//	m_shader->processShader();
//
//	cout << "Gameobject finish loading..." << endl;
//
//	loadOutlineShader();
//
//	cout << endl;
//}

GameObject::GameObject(const string& mesh_path, const vector<string>& shader_path) :
	Object(mesh_path, shader_path),
	m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_x(0), m_y(0), m_move_axis(-1)
{
	loadMesh();
	loadShader();

	loadOutlineShader();

	for (int axis = 0; axis < 3; ++axis)
	{
		shared_ptr<Gizmo> gizmo = make_shared<Gizmo>(*this, axis);
		m_gizmos.push_back(gizmo);
	}

	cout << "Gameobject successfully loaded..." << endl;	
	cout << endl;
}

GameObject::~GameObject()
{}

void GameObject::draw(glm::mat4& P, glm::mat4& V, Light& light, glm::vec3& view_pos)
{
	if (m_click)
	{
		// #1 Draw a original model
		// glStencilFunc -> stencil test, it always pass the stencil test
		// Set stencil buffer with 1 / 1 & (operation)0xFF -> 1
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		// Enable modifying stencil buffer
		glStencilMask(0xFF);

		m_shader->load();
		//glm::mat4 M = m_mesh->getTransform();
		m_shader->setVec3("object_color", m_color);
		m_shader->setLight(light);
		m_mesh->draw(P, V, *m_shader);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		m_outline_shader->load();
		m_outline_shader->setFloat("outline", 0.025f);
		m_mesh->draw(P, V, *m_outline_shader);

		// #3 Enable modifying stencil buffer
		glStencilMask(0xFF);
		// Set stencil buffer with 0 --> clear stencil buffer
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		glm::vec3 scale_factor = glm::vec3(m_mesh->getSize().x*0.1f, 
			m_mesh->getSize().y * 0.1f, m_mesh->getSize().z * 0.1f);

		for (int axis = 0; axis < 3; ++axis)
		{
			glm::mat4 M = glm::mat4(1.0f);
			M = glm::scale(glm::mat4(1.0f), scale_factor);
			m_gizmos[axis]->draw(P, V, M);
		}

		//Enable depth test
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		m_move_axis = -1;
		m_shader->load();
		glm::mat4 M = m_mesh->getTransform();
		m_shader->setPVM(P, V, M);
		m_shader->setFloat("animation", 0);
		m_shader->setVec3("object_color", m_color);
		m_shader->setVec3("viewPos", view_pos);
		m_shader->setLight(light);
		m_mesh->draw(P, V, *m_shader);
	}
}

void GameObject::loadMesh()
{
	m_mesh->processMesh();
}

void GameObject::loadOutlineShader()
{
	vector<string> outline_shader_paths = { "Shaders/Outline.vert", "Shaders/Outline.frag" };
	m_outline_shader = make_shared<Shader>(outline_shader_paths);
	m_outline_shader->processShader();
	cout << " Finish loading outline shader in gameobject" << endl;
}

bool GameObject::isGizmoClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	for (int axis = 0; axis < 3; ++axis)
	{
		if (m_gizmos.at(axis)->isClick(ray_dir, ray_pos))
		{
			m_move_axis = axis;
			//cout << "Move along axis-" << m_move_axis << endl;
			return true;
		}
	}
	m_move_axis = -1;
	return false;
}