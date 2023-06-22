#include "Object.h"

Object::Object() :
	m_mesh(nullptr), m_shader(nullptr), 
	m_click(false), m_name(""), m_position(glm::vec3(0.0f))
{}

Object::Object(const string& mesh_path) :
	m_click(false), m_name(""), m_position(glm::vec3(0.0f))
{
	int last = mesh_path.find_last_of('/');
	if (last == -1)
	{
		last = mesh_path.find_last_of('\\');
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	int length = temp.length() - temp.find_last_of('.');
	m_name = temp.substr(0, length);

	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "GameObject type : " << file_type << endl;
		if (file_type == "txt")
		{
			m_mesh = make_shared<Mesh>(m_name, mesh_path);
		}
		else if (file_type == "fbx")
		{
			m_mesh = make_shared<FBXMesh>(mesh_path);
		}
	}

	vector<string> shader_path = { "Shaders/Basic.vert", "Shaders/Basic.frag" };
	m_shader = make_shared<Shader>(shader_path);
}

Object::Object(const string& mesh_path, const vector<string>& shader) :
	m_click(false), m_name(""), m_position(glm::vec3(0.0f))
{
	int last = mesh_path.find_last_of('/');
	if (last == -1)
	{
		last = mesh_path.find_last_of('\\');
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	int length = temp.length() - temp.find_last_of('.');
	m_name = temp.substr(0, length);

	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "GameObject type : " << file_type << endl;
		if (file_type == "txt")
		{
			m_mesh = make_shared<Mesh>(m_name, mesh_path);
		}
		else if (file_type == "fbx")
		{
			m_mesh = make_shared<FBXMesh>(mesh_path);
		}
	}

	m_shader = make_shared<Shader>(shader);
}

Object::~Object()
{}

bool Object::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	return m_mesh->intersect(ray_dir, ray_pos);
}

void Object::setPosition(glm::vec3 pos)
{
	m_position += pos;
	//cout << "Set position : " << m_position << endl;
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, m_position);
	//cout << t << endl;
	m_mesh->setTransform(t);
};

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

	//M = glm::translate(M, *m_root.getPosition());
	M = m_root.getMesh()->getTransform() * M;
	
	glm::vec3 pos = glm::vec3(0.0f);
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

Grid::Grid()
{
	m_name = "Grid";
	m_mesh = make_shared<Mesh>(m_name, "Models/Plane.txt");
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

GameObject::GameObject() :
	Object(), m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path) :
	Object(mesh_path),
	m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
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

GameObject::GameObject(const string& mesh_path, const vector<string>& shader_path) :
	Object(mesh_path, shader_path),
	m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
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
		m_shader->setVec3("view_pos", view_pos);
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

		float model_size = max(m_mesh->getSize().z, max(m_mesh->getSize().x, m_mesh->getSize().y));
		glm::vec3 scale_factor1 = glm::vec3(model_size*0.1f);
		glm::vec3 scale_factor2 = glm::vec3(glm::length(m_position - view_pos)*0.2f);
		for (int axis = 0; axis < 3; ++axis)
		{
			glm::mat4 M = glm::mat4(1.0f);
			M = glm::scale(glm::mat4(1.0f), scale_factor1);
			M = glm::scale(M, scale_factor2);
			m_gizmos[axis]->draw(P, V, M);
		}

		//Enable depth test
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		m_move_axis = -1;
		m_shader->load();
		m_shader->setFloat("animation", 0);
		//m_shader->setVec3("object_color", m_color);
		m_shader->setVec3("view_pos", view_pos);
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

Geometry::Geometry() : GameObject()
{}

Geometry::~Geometry()
{}

Sphere::~Sphere()
{}

Sphere::Sphere() : 
	Geometry(), m_division(18.0f), m_radius(1.0f)
{
	cout << "Create sphere" << endl;

	m_name = "Sphere";

	vector<info::VertexLayout> layouts = calculateVertex();
	vector<unsigned int> indices = calculateIndex();
	m_mesh = make_shared<Mesh>(m_name, layouts, indices);

	vector<string> shader_path = { "Shaders/Basic.vert", "Shaders/Basic.frag" };
	m_shader = make_shared<Shader>(shader_path);
	loadShader();

	loadOutlineShader();

	for (int axis = 0; axis < 3; ++axis)
	{
		shared_ptr<Gizmo> gizmo = make_shared<Gizmo>(*this, axis);
		m_gizmos.push_back(gizmo);
	}

	cout << "Sphere successfullly loaded" << endl;
	cout << endl;
}

vector<info::VertexLayout> Sphere::calculateVertex()
{
	float angle_sector = (2 * M_PI) / m_division; // 0 to 360
	float angle_stack = M_PI / m_division; // 90 to -90
	//float length_inv = 1.0f / 

	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> texCoords;

	for (int i = 0; i <= m_division; i++) // stack
	{
		float u = (float)(M_PI / 2) - (float)angle_stack * (float)i;
		for (int j = 0; j <= m_division; j++) // Sector
		{
			float v = (float)j * angle_sector;

			float x = cos(u) * sin(v);
			float y = sin(u);
			float z = cos(u) * cos(v);

			glm::vec3 vertex = { x, y, z };
			vertices.push_back(vertex);

			glm::vec3 normal = { x, y, z };
			normals.push_back(normal);

			float s = (float)j / angle_sector;
			float t = (float)i / angle_stack;
			glm::vec2 texCoord = { s, t };
			texCoords.push_back(texCoord);
		}
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < vertices.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = vertices[i];
		layout.normal = normals[i];
		layout.texCoord = texCoords[i];

		layouts.push_back(layout);
	}

	return layouts;
}

vector<unsigned int> Sphere::calculateIndex()
{
	vector<unsigned int> indices;
	unsigned int I1;
	unsigned int I2;

	for (int i = 0; i < m_division; ++i) { // Stack
		I1 = i * (m_division + 1);
		I2 = I1 + (m_division + 1);
		
		for (int j = 0; j < m_division; ++j) { // Sector


			if (i != 0)
			{
 				indices.push_back(I1);
				indices.push_back(I2);
				indices.push_back(I1+1);
			}

			if (i != m_division - 1)
			{
				indices.push_back(I1+1);
				indices.push_back(I2);
				indices.push_back(I2+1);
			}

			I1 += 1;
			I2 += 1;
		}
	}

	return indices;
}