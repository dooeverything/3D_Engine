#include "Object.h"

#include "GameObject.h"
#include "SoftBodySolver.h"

Object::Object() :
	m_mesh(nullptr), m_shader(nullptr), m_soft(nullptr),
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(""), m_id(0)
{
	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale
}

Object::Object(const string& mesh_path) :
	m_soft(nullptr),
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(mesh_path), m_id(0)
{
	int last = int(mesh_path.find_last_of('/'));
	if (last == -1)
	{
		last = int(mesh_path.find_last_of('\\'));
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
	m_mesh->processMesh();

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();
}

Object::Object(const string& mesh_path, const vector<string>& shader) :
	m_soft(nullptr), 
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(mesh_path), m_id(0)
{
	int last = int(mesh_path.find_last_of('/'));
	if (last == -1)
	{
		last = int(mesh_path.find_last_of('\\'));
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
	m_mesh->processMesh();

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	m_shader = make_shared<Shader>(shader);
	m_shader->processShader();
}

Object::~Object()
{}

bool Object::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	if (m_mesh == nullptr) return false;

	m_click = m_mesh->intersect(ray_dir, ray_pos);
	return m_click;
}

void Object::resetRayHit()
{
	m_mesh->setRayHitMin(FLT_MAX);
}

void Object::addSoftBodySolver()
{
	m_soft = make_shared<SoftBodySolver>(m_mesh.get());
}

string Object::getIdName()
{
	string id = "";
	if (m_id)
	{
		id = to_string(m_id);
	}
	return (m_name + id);
}

void Object::setProperty(int index, const glm::vec3& t)
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

void Object::setPosition(const glm::vec3& pos)
{
	m_property[0] = pos;
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, m_property[0]);
	m_mesh->setPosition(t);
};

void Object::setRotation(const glm::vec3& rot)
{
	//cout << m_name << " : set rotation " << rot <<  endl;
	m_property[1] = rot;
	glm::mat4 t = glm::mat4(1.0f);
	
	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 axis = glm::vec3(0.0f);
		axis[i] = 1.0f;
		//cout << "Axis-" << i << " : " << endl;
		float angle = glm::radians(m_property[1][i]);
		//t = glm::rotate(t, angle, axis);
		Quaternion q;
		q.set(axis, angle);
		glm::mat4 m = q.getMatrix();
		t *= m;
		//cout << m << endl;
	}
	m_mesh->setRotation(t);
};

void Object::setScale(const glm::vec3& scale)
{
	m_property[2] = glm::abs(scale);
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::scale(t, m_property[2]);
	m_mesh->setScale(t);
}

void Object::setupFramebuffer(const glm::mat4& V, ShadowMap& depth, CubeMap& cubemap, Camera& camera)
{
	return;
}

Gizmo::Gizmo(int axis) : m_axis(axis)
{
	cout << "Gizmo constructor " << axis << endl;

	if (axis == 3)
	{
		m_mesh = make_shared<Mesh>("Gizmo_Center", "assets/models/Cube.txt");
	}
	else
	{
		m_mesh = make_shared<FBXMesh>("assets/models/Arrow.fbx");
	}
	m_mesh->processMesh();

	vector<string> shader_paths = {"assets/shaders/Arrow.vert", "assets/shaders/Arrow.frag"};
	m_shader = make_shared<Shader>(shader_paths);
	m_shader->processShader();
}

Gizmo::~Gizmo()
{}

void Gizmo::draw(GameObject& go, const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos)
{
	//cout << "Draw: " << m_axis << endl;

	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 M = glm::mat4(1.0f);
	glm::vec3 scale = glm::vec3(glm::length(m_property[0] - cam_pos) * 0.05f);
	
	if (m_axis == 3)
		scale *= 0.2f;

	M = glm::scale(M, scale);
	M = *(go.getMesh()->getPosition()) * M;
	
	if (m_axis == 1)
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	else if (m_axis == 2)
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	if (m_axis != 3)
	{
		if (m_click)
			color[m_axis] = 0.3f;
		else
			color[m_axis] = 0.7f;	
	}
	else
	{
		color = glm::vec3(1.0f);
	}
	
	m_mesh->setPosition(M);

	m_shader->load();
	m_shader->setVec3("object_color", color);
	m_mesh->draw(P, V, *m_shader);
}

bool Gizmo::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	if (m_axis == 3) return false;

	return m_mesh->intersect(ray_dir, ray_pos);
}

Grid::Grid()
{
	m_name = "Grid";
	m_mesh = make_shared<Mesh>(m_name, "assets/models/Grid.txt");
	m_mesh->processMesh();
	
	vector<string> shader_path = { "assets/shaders/Grid.vert", "assets/shaders/Grid.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	cout << "Grid finish loading..." << endl;
	cout << endl;
}

void Grid::draw(const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos)
{
	m_shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	m_shader->setPVM(P, V, M);
	m_shader->setVec3("cam_pos", cam_pos);
	m_mesh->draw();
}

