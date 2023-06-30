#include "Object.h"

Object::Object() :
	m_mesh(nullptr), m_shader(nullptr), 
	m_click(false), m_name(""), m_property({})
{
	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale
}

Object::Object(const string& mesh_path) :
	m_click(false), m_name(""), m_property({})
{
	int last = mesh_path.find_last_of('/');
	if (last == -1)
	{
		last = mesh_path.find_last_of('\\');
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	m_name = temp.substr(0, temp.find_last_of('.'));

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
	else
	{
		cerr << "Invalid file " << mesh_path << endl;
		assert(0);
	}

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	vector<string> shader_path = { "Shaders/Basic.vert", "Shaders/Basic.frag" };
	m_shader = make_shared<Shader>(shader_path);
}

Object::Object(const string& mesh_path, const vector<string>& shader) :
	m_click(false), m_name(""), m_property({})
{
	int last = mesh_path.find_last_of('/');
	if (last == -1)
	{
		last = mesh_path.find_last_of('\\');
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	m_name = temp.substr(0, temp.find_last_of('.'));

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

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	m_shader = make_shared<Shader>(shader);
}

Object::~Object()
{}

bool Object::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	return m_mesh->intersect(ray_dir, ray_pos);
}

void Object::setProperty(int index, glm::vec3 t)
{
	switch (index)
	{
	case(0):
		// Set Position
		setPosition(t);
		break;

	case(1):
		// Set Rotation
		setRotation(t);
		break;

	case(2):
		setScale(t);
		break;

	default:
		cerr << "Wrong property" << endl;
		assert(0);
		break;
	}
}

void Object::setPosition(glm::vec3 pos)
{
	m_property[0] = pos;
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, m_property[0]);
	m_mesh->setPosition(t);
};

void Object::setRotation(glm::vec3 rot)
{
	m_property[1] = rot;
	glm::mat4 t = glm::mat4(1.0f);
	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 axis = glm::vec3(0.0f);
		axis[i] = 1.0f;
		float angle = m_property[1][i];
		t = glm::rotate(t, angle, axis);
	}
	m_mesh->setRotation(t);
};

void Object::setScale(glm::vec3 scale)
{
	m_property[2] = glm::abs(scale);
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::scale(t, m_property[2]);
	m_mesh->setScale(t);
};

ShadowMap::ShadowMap() :
	m_shadow_buffer(shared_ptr<ShadowBuffer>()),
	m_proj(glm::mat4(0.0f)), m_view(glm::mat4(0.0f)), m_light_position(glm::vec3(0.0f))
{}

ShadowMap::ShadowMap(glm::vec3 position)
{
	cout << "Shadw map constructor" << endl;
	m_light_position = position;
	m_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20.0f);
	m_view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0, -1.0, 0.0));

	m_shadow_buffer = make_shared<ShadowBuffer>();
	m_shadow_buffer->createBuffers(WIDTH, HEIGHT);

	vector<string> shader_path = { "Shaders/ShadowMap.vert", "Shaders/ShadowMap.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();
}

ShadowMap::~ShadowMap() {}

void ShadowMap::draw(vector<shared_ptr<GameObject>>& gameobjects)
{
	glViewport(0, 0, WIDTH, HEIGHT);
	m_shadow_buffer->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		for (auto& it : gameobjects)
		{
			m_shader->load();
			it->getMesh()->draw(m_proj, m_view, *m_shader);
		}
	m_shadow_buffer->unbind();
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

	M = *(m_root.getMesh()->getPosition()) * M;

	glm::vec3 pos = glm::vec3(0.0f);
	pos[m_axis] = 1.5f;
	M = glm::translate(M, pos);

	if (m_axis == 1)
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	else if (m_axis == 2)
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	if (m_click)
		color[m_axis] = 0.5f;
	else
		color[m_axis] = 1.0f;
	
	m_mesh->setPosition(M);

	m_shader->load();
	m_shader->setVec3("object_color", color);
	m_shader->setPVM(P, V, M);
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

GameObject::GameObject() : Object(), 
	m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path) : Object(mesh_path),
	m_gizmos({}), m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{
	loadMesh();
	loadShader();

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

void GameObject::draw(glm::mat4& P, glm::mat4& V, 
	Light& light, glm::vec3& view_pos, ShadowMap& shadow)
{
	if(!m_click)
	{
		m_move_axis = -1;
		//m_outline_buffer = nullptr;
	}
	
	m_shader->load();
	glm::mat4 shadow_proj = (*shadow.getProj()) * (*shadow.getView());
	m_shader->setMat4("light_matrix", shadow_proj);
	m_shader->setVec3("light_pos", *shadow.getPosition());
	m_shader->setFloat("animation", 0);
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(light);

	m_shader->setFloat("width", m_screen_w);
	m_shader->setFloat("height", m_screen_h);

	// Load shadow map as texture
	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shadow.getBuffer().bindFrameTexture();
	m_mesh->draw(P, V, *m_shader);
}

void GameObject::drawGizmos(glm::mat4& P, glm::mat4& V, glm::vec3& view_pos)
{
	glm::vec3 scale_factor2 = glm::vec3(glm::length(m_property[0] - view_pos) * 0.05f);
	for (int axis = 0; axis < 3; ++axis)
	{
		glm::mat4 M = glm::mat4(1.0f);
		M = glm::scale(M, scale_factor2);
		m_gizmos[axis]->draw(P, V, M);
	}
}

void GameObject::loadMesh()
{
	m_mesh->processMesh();
}

bool GameObject::isGizmoClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	for (int axis = 0; axis < 3; ++axis)
	{
		if (m_gizmos.at(axis)->isClick(ray_dir, ray_pos))
		{
			m_move_axis = axis;
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

Outline::Outline(int width, int height)
{
	vector<string> outline_shader_paths = { "Shaders/Outline.vert", "Shaders/Outline.frag" };
	m_outline_shader = make_unique<Shader>(outline_shader_paths);
	m_outline_shader->processShader();

	vector<string> mask_shader_paths = { "Shaders/Mask.vert", "Shaders/Mask.frag" };
	m_mask_shader = make_unique<Shader>(mask_shader_paths);
	m_mask_shader->processShader();

	for (int i = 0; i < 4; ++i)
	{
		m_outline_buffers.push_back(make_unique<FrameBuffer>());
		m_outline_buffers.at(i)->createBuffers(width, height);
	}

	vector<string> debug_shader = { "Shaders/Debug.vert", "Shaders/Debug.frag" };
	m_debug = make_unique<GameObject>("Models/Debug.txt", debug_shader);
}

Outline::~Outline()
{}

void Outline::setupBuffers(GameObject& go, glm::mat4 & P, glm::mat4 & V)
{
	m_outline_buffers.at(0)->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_mask_shader->load();
		go.getMesh()->draw(P, V, *m_mask_shader);
	m_outline_buffers.at(0)->unbind();
	m_debug->setProperty(2, glm::vec3(1.0f));
	m_outline_buffers.at(1)->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_debug->getShader()->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(0)->bindFrameTexture();
		m_debug->getMesh()->draw();
	m_outline_buffers.at(1)->unbind();

	m_outline_buffers.at(2)->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_outline_shader->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(1)->bindFrameTexture();
		m_outline_shader->setInt("outline_map", 0);
		m_outline_shader->setFloat("width", 1400);
		m_outline_shader->setFloat("height", 800);
		m_outline_shader->setInt("pass", 0);
		m_outline_shader->setFloat("jump", 1.0);
		m_debug->getMesh()->draw();
		//go.setProperty(2, glm::vec3(1.05f, 1.05f, 1.05f));
		//go.getMesh()->draw(P, V, *m_outline_shader);
	m_outline_buffers.at(2)->unbind();

	m_outline_buffers.at(3)->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_outline_shader->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(2)->bindFrameTexture();
		m_outline_shader->setInt("outline_map", 0);
		m_outline_shader->setFloat("width", 1400);
		m_outline_shader->setFloat("height", 800);
		m_outline_shader->setInt("pass", 1);
		m_outline_shader->setFloat("jump", 2);
		m_debug->getMesh()->draw();
	m_outline_buffers.at(3)->unbind();
}

void Outline::draw(GameObject& go, glm::mat4& P, glm::mat4& V)
{
	m_outline_shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.back()->bindFrameTexture();
	m_outline_shader->setInt("outline_map", 0);
	glActiveTexture(GL_TEXTURE0+1);
	m_outline_buffers.at(0)->bindFrameTexture();
	m_outline_shader->setInt("mask_map", 1);
	m_outline_shader->setFloat("width", 1400);
	m_outline_shader->setFloat("height", 800);
	m_outline_shader->setInt("pass", 2);
	m_debug->setProperty(2, go.getMesh()->getSize());
	m_debug->getMesh()->draw();
}

