#include "MeshImporter.h"

#include <fstream>
#include <sstream>

#include "Material.h"
#include "Mesh.h"

MeshImporter::MeshImporter() {}

MeshImporter::~MeshImporter() {}

shared_ptr<MeshImporter> MeshImporter::create(const string& path)
{
	string::size_type pos = path.find_last_of('.');
	shared_ptr<MeshImporter> importer = nullptr;
	if (pos != path.length())
	{
		string file_type = path.substr(pos + 1, path.length());
		cout << "Import: " << file_type << endl;
		if (file_type == "txt")
		{
			importer = make_shared<TXTImporter>(path);
		}
		else if (file_type == "fbx")
		{
			importer = make_shared<FBXImporter>(path);
		}
		//else if (file_type == "obj")
		//{
		//	importer = make_shared<OBJImporter>(path);
		//}
	}
	else
	{
		cerr << "Invalid file " << path << endl;
		assert(0);
	}

	if (importer == nullptr)
	{
		cout << "Mesh importer error" << endl;
		assert(0);
	}

	return importer;
}

string MeshImporter::findName(const string& path)
{
	int last = int(path.find_last_of('/'));
	if (last == -1)
	{
		last = int(path.find_last_of('\\'));
	}
	string temp = path.substr(last + 1, path.length());
	string name = temp.substr(0, temp.find_last_of('.'));

	return name;
}

TXTImporter::TXTImporter(const string& path) : m_path(path) {}

TXTImporter::~TXTImporter() {}

void TXTImporter::importMesh(shared_ptr<Mesh>& mesh)
{
	readFile();
	shared_ptr<Mesh> m = make_shared<Mesh>(MeshImporter::findName(m_path));
	m->setupBuffer(m_layouts, m_indices);
	mesh = m;
}

void TXTImporter::importMesh(vector<shared_ptr<Mesh>>& meshes)
{
	readFile();
	shared_ptr<Mesh> m = make_shared<Mesh>(MeshImporter::findName(m_path));
	m->setupBuffer(m_layouts, m_indices);
	meshes.emplace_back(m);
}

void TXTImporter::readFile()
{
	vector<info::VertexLayout> layouts;
	vector<unsigned int> indices;

	ifstream vertex_file(m_path);

	cout << "Process mesh from text file: " << m_path << endl;

	if (!vertex_file.is_open())
	{
		cout << "Failed to open the vertex file " << m_path << endl;
		assert(0);
	}

	string line;
	string type, x, y, z;

	vector<glm::vec3> positions;
	vector<glm::vec3> normals;
	vector<glm::vec2> texCoords;

	while (getline(vertex_file, line))
	{
		if (line.empty()) continue;

		stringstream ss(line);

		getline(ss, type, ' ');

		if (type == "#") continue;

		if (type == "v") // vertex coordinates
		{
			getline(ss, x, ' ');
			getline(ss, y, ' ');
			getline(ss, z, ' ');

			glm::vec3 position = { stof(x), stof(y), stof(z) };
			positions.push_back(position);
		}
		else if (type == "vn")
		{
			getline(ss, x, ' ');
			getline(ss, y, ' ');
			getline(ss, z, ' ');

			glm::vec3 normal = { stof(x), stof(y), stof(z) };
			normals.push_back(normal);
		}
		else if (type == "vt")
		{
			getline(ss, x, ' ');
			getline(ss, y, ' ');

			glm::vec2 texCoord = { stof(x), stof(y) };
			texCoords.push_back(texCoord);
		}
		else if (type == "f")
		{
			string index;
			while (getline(ss, index, ','))
			{
				unsigned int index_uint = stoul(index, nullptr, 0);
				indices.push_back(index_uint);
			}
		}
	}

	for (int i = 0; i < positions.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = positions[i];

		if (normals.size() > 0)
			layout.normal = normals[i];

		if (texCoords.size() > 0)
			layout.texCoord = texCoords[i];

		layouts.push_back(layout);
	}

	m_layouts = layouts;
	m_indices = indices;
}

FBXImporter::FBXImporter(const string& path) :
	m_aiScene(nullptr), m_importer(), m_textures_loaded({}),
	m_path(path), m_directory("") {}

FBXImporter::~FBXImporter()
{
	//delete m_aiScene;
}

void FBXImporter::importMesh(vector<shared_ptr<Mesh>>& meshes)
{
	cout << "Process mesh from fbx file: " << m_path << endl;

	m_aiScene = m_importer.ReadFile(
		m_path,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
	);

	if (!m_aiScene || m_aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_aiScene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << m_importer.GetErrorString() << endl;
		assert(0);
	}

	// The directory path of the fbx file 
	int last = int(m_path.find_last_of('/'));
	if (last == -1)
	{
		last = int(m_path.find_last_of("\\"));
	}
	m_directory = m_path.substr(0, last);

	// Process ASSIMP's root node, and then recursively process its child node
	aiNode* root_node = m_aiScene->mRootNode;
	processNode(root_node, m_aiScene, meshes);
}

void FBXImporter::importMesh(shared_ptr<Mesh>& mesh)
{
	cout << "Cannot call this function" << endl;
	assert(0);

	return;
}

void FBXImporter::processNode(
	const aiNode* node, 
	const aiScene* scene, 
	vector<shared_ptr<Mesh>>& meshes)
{
	cout << " -Process aiNode " << node->mName.C_Str() << " meshes: " << node->mNumMeshes << " childrens: " << node->mNumChildren << endl;

	// Node Transformation:
	// coordinates of vertices of meshes from a node --> model space coordinate
	// Ends up whole nodes placed at correct position 
	glm::mat4 node_transformation = ConvertMatrixToGLMFormat(node->mTransformation);

	glm::mat4 adjust = { 1.0f,  0.0f,  0.0f, 0.0f,
						0.0f,  0.0f, -1.0f, 0.0f,
						0.0f,  1.0f,  0.0f, 0.0f,
						0.0f,  0.0f,  0.0f, 1.0f };

	// Loop through each mesh along the node and load it to the framebuffer
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
		shared_ptr<VertexBuffer> buffer = processBuffer(ai_mesh, scene, node->mName.C_Str(), node_transformation);
		shared_ptr<Material> material = processMaterial(ai_mesh, scene);
		processTextures(ai_mesh, scene, material.get());

		shared_ptr<Mesh> mesh = make_shared<Mesh>(node->mName.C_Str(), buffer, material);
		meshes.emplace_back(mesh);

		cout << "push mesh from fbx" << endl;
	}

	// Process all the children node recursively to process each mesh's children nodes
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		aiNode* child_node = node->mChildren[i];
		processNode(child_node, scene, meshes);
	}
}

shared_ptr<VertexBuffer> FBXImporter::processBuffer(
	const aiMesh* mesh, const aiScene* scene, const string& name, const glm::mat4& m)
{
	cout << "Process Buffer" << endl;

	vector<info::VertexLayout> layouts;
	vector<info::uint> indices;

	vector<glm::vec3> positions;
	vector<glm::vec3> normals;
	vector<glm::vec2> texCoords;

	// Get info for mesh
	unsigned int num_vertices = mesh->mNumVertices;
	unsigned int num_faces = mesh->mNumFaces;
	unsigned int num_indices = num_faces * 3;
	unsigned int num_bones = mesh->mNumBones;
	string mesh_name = mesh->mName.C_Str();

	for (unsigned int i = 0; i < num_vertices; ++i)
	{
		glm::vec3 position;
		// Convert Assimp::vec3 to glm::vec3
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;

		// Set position of each vertex
		positions.push_back(position);

		//  Set normal of each vertex
		if (mesh->HasNormals())
		{
			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
			normals.push_back(normal);
		}

		// set texture coordinate of each vertex
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 texCoord;
			texCoord.x = mesh->mTextureCoords[0][i].x;
			texCoord.y = mesh->mTextureCoords[0][i].y;
			texCoords.push_back(texCoord);
		}
		else
		{
			texCoords.push_back(glm::vec2(0.0f, 0.0f));
		}
	}

	for (int i = 0; i < positions.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = positions[i];
		layout.normal = normals[i];
		layout.texCoord = texCoords[i];
		layouts.push_back(layout);
	}

	// Get indices of the mesh faces(triangle)
	for (unsigned int i = 0; i < num_faces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	shared_ptr<VertexBuffer> buffer = make_shared<VertexBuffer>();
	buffer->createBuffers(layouts, indices);
	return buffer;
}

shared_ptr<Material> FBXImporter::processMaterial(const aiMesh* mesh, const aiScene* scene)
{
	cout << "Process Material" << endl;

	// Process materials
	shared_ptr<aiMaterial> ai_material = make_shared<aiMaterial>();
	ai_material->CopyPropertyList(ai_material.get(), scene->mMaterials[mesh->mMaterialIndex]);

	aiString str;

	if (AI_SUCCESS != ai_material->Get(AI_MATKEY_NAME, str))
	{
		// if material does not have any name just return empty Material...
		cerr << "Material does not have any name..." << endl;
		assert(0);
	}

	aiColor3D color(0.0f, 0.0f, 0.0f);

	shared_ptr<Material> material = make_shared<Material>();
	if (AI_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_AMBIENT, color))
	{
		material->setBaseColor({ color.r, color.g, color.b });
	}
	else
	{
		material->setBaseColor(glm::vec3(0.0f));
	}

	float metal_factor = 0.0;
	if (AI_SUCCESS == ai_material->Get(AI_MATKEY_SHININESS, metal_factor))
	{
		material->setMetallic(metal_factor);
	}

	return material;
}

void FBXImporter::processTextures(const aiMesh* mesh, const aiScene* scene, Material* material)
{
	cout << "Process Textures " << endl;

	shared_ptr<aiMaterial> ai_material = make_shared<aiMaterial>();
	ai_material->CopyPropertyList(ai_material.get(), scene->mMaterials[mesh->mMaterialIndex]);

	vector<shared_ptr<Texture>> diffuses = loadTexture(ai_material, aiTextureType_DIFFUSE, "color");
	material->addTextureBase(diffuses);

	vector<shared_ptr<Texture>> speculars = loadTexture(ai_material, aiTextureType_SPECULAR, "specular");
	material->addTextureSpecular(speculars);

	vector<shared_ptr<Texture>> normals = loadTexture(ai_material, aiTextureType_HEIGHT, "normal");
	material->addTextureNormal(normals);

	vector<shared_ptr<Texture>> heights = loadTexture(ai_material, aiTextureType_AMBIENT, "ambient");
	material->addTextureHeight(heights);
}

vector<shared_ptr<Texture>> FBXImporter::loadTexture(shared_ptr<aiMaterial> mat, aiTextureType type, string typeName)
{
	vector<shared_ptr<Texture>> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		string path = m_directory + '/' + str.C_Str();
		cout << "Path: " << m_directory << endl;
		// Check if the texture was loaded before
		for (unsigned int j = 0; j < m_textures_loaded.size(); ++j)
		{
			if (strcmp(m_textures_loaded[j]->getPath().data(), path.c_str()) == 0)
			{
				// If the texture was loaded before, 
				// just push the loaded texture to the textures container
				textures.push_back(m_textures_loaded[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			shared_ptr<Texture> new_texture = make_shared<Texture>(path, typeName);
			new_texture->loadTexture();
			textures.push_back(new_texture);
			m_textures_loaded.push_back(new_texture);
		}
	}
	return textures;
}

glm::mat4 FBXImporter::ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat)
{
	glm::mat4 mat;

	mat[0][0] = aiMat.a1;
	mat[1][0] = aiMat.a2;
	mat[2][0] = aiMat.a3;
	mat[3][0] = aiMat.a4;

	mat[0][1] = aiMat.b1;
	mat[1][1] = aiMat.b2;
	mat[2][1] = aiMat.b3;
	mat[3][1] = aiMat.b4;

	mat[0][2] = aiMat.c1;
	mat[1][2] = aiMat.c2;
	mat[2][2] = aiMat.c3;
	mat[3][2] = aiMat.c4;

	mat[0][3] = aiMat.d1;
	mat[1][3] = aiMat.d2;
	mat[2][3] = aiMat.d3;
	mat[3][3] = aiMat.d4;

	return mat;

}



