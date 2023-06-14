#include "FileLoader.h"

FileLoader::FileLoader() : m_path("") {}

FileLoader::~FileLoader() {}

TXTLoader::TXTLoader() : FileLoader(), m_mesh(make_unique<Mesh>()) {}

TXTLoader::~TXTLoader() {}

void TXTLoader::loadMesh(const string& file_path)
{
	m_path = file_path;
	m_mesh->loadTXT(file_path);
}

FBXLoader::FBXLoader() : m_mesh(make_unique<FBXMesh>()), 
						 m_aiScene(nullptr), m_importer() {}

FBXLoader::~FBXLoader() {}

void FBXLoader::loadMesh(const string& file_path)
{
	cout << "Load mesh from fbx file: " << file_path << endl;
	m_path = file_path;
	m_aiScene = m_importer.ReadFile(file_path, aiProcess_Triangulate |
											   aiProcess_GenSmoothNormals |
											   aiProcess_CalcTangentSpace);

	if (!m_aiScene || m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_aiScene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << m_importer.GetErrorString() << endl;
		assert(0);
	}

	// The directory path of the file path
	string directory = file_path.substr(0, file_path.find_last_of('/'));
	m_mesh->setDirectory(directory);

	// Process ASSIMP's root node, and then recursively process its child node
	aiNode* root_node = m_aiScene->mRootNode;
	m_mesh->processNode(root_node, m_aiScene);
}