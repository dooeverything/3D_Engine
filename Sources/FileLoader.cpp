#include "FileLoader.h"

Object::Object() : m_shader(make_shared<Shader>())
{}

Object::Object(const vector<string>& shader)
	: m_shader(make_shared<Shader>(shader))
{}

Object::~Object()
{}

void Object::loadObject()
{
	loadMesh();
	loadShader();
}

void Object::loadShader()
{
	m_shader->loadShaderFile();
}

Gizmo::Gizmo() : m_axis_mesh({}), m_root(nullptr), m_clicked_axis(-1)
{}

Gizmo::Gizmo(shared_ptr<Object> root) : m_axis_mesh({}), m_root(root), m_clicked_axis(-1)
{}

Gizmo::~Gizmo()
{}

void Gizmo::draw(glm::mat4 P, glm::mat4 V)
{
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	for (int axis = 0; axis < 3; ++axis)
	{
		m_shader->load();
		glm::mat4 M = m_root->getMesh()->getTransform();
		M = glm::translate(M, m_root->getMesh()->getCenter());
		m_shader->setPVM(P, V, M);
		
		if (m_clicked_axis == axis)
			color[axis] = 0.5f;
		else 
			color[axis] = 1.0f;
		
		m_shader->setVec3("object_color", color);
		m_axis_mesh[axis]->draw();
	}
}

void Gizmo::isPicking(int w, int h, glm::vec3 ray_dir, glm::vec3 ray_pos)
{
	int axis = 0;
	for (; axis < 3; ++axis)
	{
		if (m_axis_mesh[axis]->intersect(ray_dir, ray_pos))
			break;
	}
	m_clicked_axis = axis;
}

GameObject::GameObject() : Object(), m_mesh(make_shared<Mesh>())
{}

GameObject::GameObject(string& mesh_path, const vector<string>& shader_path)
	: Object(shader_path), m_mesh(make_shared<Mesh>(mesh_path))
{}

GameObject::~GameObject()
{}

void GameObject::loadMesh()
{
	m_mesh->processMesh();
}

FBXObject::FBXObject(const string& mesh_path, const vector<string>& shader_path)
	: Object(shader_path), m_mesh(make_shared<FBXMesh>()),
	  m_aiScene(nullptr), m_importer()
{}

FBXObject::FBXObject()
	: m_mesh(make_shared<FBXMesh>()), m_aiScene(nullptr), m_importer()
{}

FBXObject::~FBXObject()
{}

void FBXObject::loadMesh()
{
	cout << "Load mesh from fbx file: " << m_mesh->getPath() << endl;

	m_aiScene = m_importer.ReadFile
	(
		m_mesh->getPath(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
	);

	if (!m_aiScene || m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_aiScene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << m_importer.GetErrorString() << endl;
		assert(0);
	}

	// The directory path of the file path
	string directory = m_mesh->getPath().substr(0, m_mesh->getPath().find_last_of('/'));
	m_mesh->setDirectory(directory);

	// Process ASSIMP's root node, and then recursively process its child node
	aiNode* root_node = m_aiScene->mRootNode;
	m_mesh->processNode(root_node, m_aiScene);
}