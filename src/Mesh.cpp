#include "Mesh.h"
#include <fstream>
#include <sstream>

#include "Texture.h"
#include "Material.h"

BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : m_min(min), m_max(max)
{}

BoundingBox::~BoundingBox() {}

bool BoundingBox::intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos, float& t_min, float& t_max)
{
	float temp;
	float tmin = -1.0e10;
	float tmax = 1.0e10;

	// Loop through each axis of the ray
	for (int i = 0; i < 3; ++i)
	{
		float rd = ray_dir[i];

		if (abs(rd) <= 5e-3)
		{
			// if the ray is parallel to the plane, zero division
			continue; 
		}

		float v1 = m_min[i] - ray_pos[i];
		float v2 = m_max[i] - ray_pos[i];

		float t1 = v1 / rd;
		float t2 = v2 / rd;

		if (t1 > t2)
		{
			temp = t1;
			t1 = t2;
			t2 = temp;
		}

		if (t1 > tmin)
			tmin = t1;
		if (t2 < tmax)
			tmax = t2;
	}

	if (tmin > tmax) // if the box is missed
	{
		//cout << "Object is missed" << endl;
		return false;
	}

	if (tmax < 0.0f) // if the box is behind the ray
	{
		cout << "Object is behind the scene" << endl;
		return false;
	}

	t_min = tmin;
	t_max = tmax;

	return true;
}

void BoundingBox::draw()
{
	glm::vec3 size = m_max - m_min;
	glm::vec3 center = glm::vec3((m_max.x + m_min.x) / 2, (m_max.y + m_min.y) / 2, (m_max.z + m_min.z) / 2);
	glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);
}

Mesh::Mesh(string name) :
	m_directory(""), m_name(name), m_buffer(make_unique<VertexBuffer>()),
	m_textures({}), m_material(make_unique<Material>()),
	m_transform_pos(glm::mat4(1.0f)), m_transform_rot(glm::mat4(1.0f)),
	m_transform_scale(glm::mat4(1.0f)), m_center(glm::vec3(0.0f)),
	t_min(FLT_MAX), t_max(FLT_MIN)
{}

Mesh::Mesh(string name, const string& path) : 
	m_directory(path), m_name(name), m_buffer(make_unique<VertexBuffer>()),
	m_textures({}), m_material(make_unique<Material>()), 
	m_transform_pos(glm::mat4(1.0f)), m_transform_rot(glm::mat4(1.0f)),
	m_transform_scale(glm::mat4(1.0f)), m_center(glm::vec3(0.0f)),
	t_min(FLT_MAX), t_max(FLT_MIN)
{
}

// Call from FBXMesh
Mesh::Mesh(string name, shared_ptr<VertexBuffer> buffer, 
		   vector<shared_ptr<Texture>> textures, 
		   shared_ptr<Material> material) :
	m_name(name), m_bbox(), 
	m_textures(textures), m_buffer(buffer), m_material(material),
	m_transform_pos(glm::mat4(1.0f)), m_transform_rot(glm::mat4(1.0f)),
	m_transform_scale(glm::mat4(1.0f)), m_center(glm::vec3(0.0f)),
	t_min(FLT_MAX), t_max(FLT_MIN)
{
	computeBoundingBox();
}

void Mesh::processMesh()
{
	vector<info::VertexLayout> layouts;
	vector<unsigned int> indices;

	ifstream vertex_file(m_directory);

	cout << "Load mesh from text file: " << m_directory << endl;

	if (!vertex_file.is_open())
	{
		cout << "Failed to open the vertex file " << m_directory << endl;
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
		
		if(normals.size() > 0)
			layout.normal = normals[i];
		
		if(texCoords.size() > 0)
			layout.texCoord = texCoords[i];

		layouts.push_back(layout);
	}

	m_buffer->createBuffers(layouts, indices);
	computeBoundingBox();
}

void Mesh::drawArrays()
{
	m_buffer->bind();
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(m_buffer->getLayouts().size()));
	m_buffer->unbind();
}

void Mesh::drawInstance(glm::mat4& P, glm::mat4& V)
{
	m_buffer->bind();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawElementsInstanced(GL_POINTS, m_buffer->getSizeOfIndices(), GL_UNSIGNED_INT, 0, m_buffer->getSizeOfInstance());
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	m_buffer->unbind();
}

void Mesh::draw()
{
	m_buffer->bind();
	glDrawElements(GL_TRIANGLES, m_buffer->getSizeOfIndices(), GL_UNSIGNED_INT, nullptr);
	m_buffer->unbind();
}

void Mesh::draw(const glm::mat4& P, const glm::mat4& V, Shader& shader, bool terrain)
{
	unsigned int color_index = 1;
	unsigned int specular_index = 1;
	unsigned int normal_index = 1;
	unsigned int height_index = 1;

	glm::mat4 adjust = glm::mat4(1.0f);
	shader.load();
	shader.setMat4("adjust", adjust);
	shader.setMaterial(*m_material);

	glm::mat4 M = m_transform_pos * m_transform_rot * m_transform_scale;
	shader.setPVM(P, V, M);
	shader.setInt("type", 0);

	if (m_material->getTexture() != nullptr)
	{
		shader.setInt("type", 1);
		shader.setInt("texture_map", 4);
		glActiveTexture(GL_TEXTURE0 + 4);
		m_material->getTexture()->setActive();
	}

	// Set texture before draw a mesh
	if (m_textures.size() > 0)
	{
		shader.setInt("type", 2);
		for (int i = 0; i < m_textures.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i + 4);

			string index;
			string type = m_textures[i]->getType();
			if (type == "color")
				index = to_string(color_index++);

			shader.setInt(("tex_fbx." + type + index).c_str(), i+4);
			m_textures[i]->setActive();
		}	
	}

	if (terrain)
		drawArrays();
	else
		draw();

	// Set everything back to default texture
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::drawLowQuality(Shader& shader)
{
	shader.load();
	shader.setMaterial(*m_material);
	glm::mat4 M = m_transform_pos * m_transform_rot * m_transform_scale;
	shader.setMat4("model", M);
	draw();
}

bool Mesh::intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos)
{
	t_min;
	t_max;
	computeBoundingBox();
	return m_bbox->intersect(ray_dir, ray_pos, t_min, t_max);
}

void Mesh::computeBoundingBox()
{
	glm::vec3 min;
	glm::vec3 max;
	//cout << "Computer Bounding box " << m_name << endl;

	vector<glm::vec3> positions;
	glm::mat4 t = m_transform_pos * m_transform_rot * m_transform_scale;
	for (auto& it : m_buffer->getLayouts())
	{
		glm::vec4 p = glm::vec4(it.position, 1.0f);
		p = t * p;
		positions.push_back(glm::vec3(p.x, p.y, p.z));
	}

	min = max = positions.at(0); 

	for (auto& it : positions)
	{
		if (it.x < min.x) min.x = it.x;
		if (it.x > max.x) max.x = it.x;

		if (it.y < min.y) min.y = it.y;
		if (it.y > max.y) max.y = it.y;

		if (it.z < min.z) min.z = it.z;
		if (it.z > max.z) max.z = it.z;
	}
	m_center = glm::vec3((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2);	
	m_bbox = make_shared<BoundingBox>(min, max);
}

FBXMesh::FBXMesh() : Mesh(""),
	m_meshes({}), m_textures_loaded({}), m_path(""),
	  m_aiScene(nullptr), m_importer()
{}

FBXMesh::FBXMesh(const string& path) : Mesh(""),
	m_meshes({}), m_textures_loaded({}), m_path(path),
	  m_aiScene(nullptr), m_importer()
{}

FBXMesh::~FBXMesh() 
{}

void FBXMesh::processMesh()
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
	
	cout << "Process mesh: " << m_directory.size() << " last: " << last << endl;
	
	// Process ASSIMP's root node, and then recursively process its child node
	aiNode* root_node = m_aiScene->mRootNode;
	processNode(root_node, m_aiScene);
}

void FBXMesh::processNode(const aiNode* node, const aiScene* scene)
{
	cout << " -Process aiNode " << node->mName.C_Str() << " meshes: " << node->mNumMeshes << " childrens: " << node->mNumChildren << endl;

	// Node Transformation:
	// coordinates of vertices of meshes from a node --> model space coordinate
	// Ends up whole nodes placed at correct position 
	glm::mat4 node_transformation = ConvertMatrixToGLMFormat(node->mTransformation);

	glm::mat4 adjust = {1.0f,  0.0f,  0.0f, 0.0f,
						0.0f,  0.0f, -1.0f, 0.0f,
						0.0f,  1.0f,  0.0f, 0.0f,
						0.0f,  0.0f,  0.0f, 1.0f};

	// Loop through each mesh along the node and load it to the framebuffer
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
		shared_ptr<VertexBuffer> buffer = processBuffer(ai_mesh, scene, node->mName.C_Str(), node_transformation);
		shared_ptr<Material> material = processMaterial(ai_mesh, scene);
		vector<shared_ptr<Texture>> texture = processTextures(ai_mesh, scene);

		shared_ptr<Mesh> mesh = make_shared<Mesh>(node->mName.C_Str(), buffer, texture, material);
		m_meshes.push_back(mesh);
	}

	// Process all the children node recursively to process each mesh's children nodes
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		aiNode* child_node = node->mChildren[i];
		processNode(child_node, scene);
	}
}

shared_ptr<VertexBuffer> FBXMesh::processBuffer(const aiMesh* mesh, const aiScene* scene, const string& name, glm::mat4& m)
{
	cout << "  Process fbx Buffer: " << name << endl;

	vector<info::VertexLayout> layouts;
	vector<unsigned int> indices;

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

shared_ptr<Material> FBXMesh::processMaterial(const aiMesh* mesh, const aiScene* scene)
{
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

vector<shared_ptr<Texture>> FBXMesh::processTextures(const aiMesh* mesh, const aiScene* scene)
{
	// Process materials
	shared_ptr<aiMaterial> ai_material = make_shared<aiMaterial>();
	ai_material->CopyPropertyList(ai_material.get(), scene->mMaterials[mesh->mMaterialIndex]);

	vector<shared_ptr<Texture>> textures;
	
	// Load Diffuse from texture
	vector<shared_ptr<Texture>> diffuse_maps = loadTexture(ai_material, aiTextureType_DIFFUSE, "color");
	textures.insert(textures.end(), make_move_iterator(diffuse_maps.begin()), make_move_iterator(diffuse_maps.end()));

	// Load specular from texture
	vector<shared_ptr<Texture>> specularMaps = loadTexture(ai_material, aiTextureType_SPECULAR, "specular");
	textures.insert(textures.end(), make_move_iterator(specularMaps.begin()), make_move_iterator(specularMaps.end()));

	// Load normal map from texture
	vector<shared_ptr<Texture>> normalMaps = loadTexture(ai_material, aiTextureType_HEIGHT, "normal");
	textures.insert(textures.end(), make_move_iterator(normalMaps.begin()), make_move_iterator(normalMaps.end()));

	// Load height map from texture
	vector<shared_ptr<Texture>> heightMaps = loadTexture(ai_material, aiTextureType_AMBIENT, "ambient");
	textures.insert(textures.end(), make_move_iterator(heightMaps.begin()), make_move_iterator(heightMaps.end()));

	return textures;
}

vector<shared_ptr<Texture>> FBXMesh::loadTexture(shared_ptr<aiMaterial> mat, 
											  aiTextureType type, string typeName)
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

void FBXMesh::draw(const glm::mat4& P, const glm::mat4& V, Shader& shader, bool terrain)
{
	for (unsigned int i = 0; i < m_meshes.size(); ++i)
		m_meshes[i]->draw(P, V, shader, terrain);
}

void FBXMesh::draw()
{
	for (unsigned int i = 0; i < m_meshes.size(); ++i)
		m_meshes[i]->draw();
}

bool FBXMesh::intersect(const glm::vec3& ray_dir, const glm::vec3& ray_pos)
{
	//cout << "Intersect in FBX Mesh" << endl;
	bool inter = false;
	for (auto& it : m_meshes)
	{
		//cout << "Intersect with mesh " << it->getName() << endl;
		if (it->intersect(ray_dir, ray_pos))
		{
			inter = true;
			break;
		}
	}

	return inter;
}

void FBXMesh::setPosition(glm::mat4 t)
{
	for (auto& it : m_meshes)
		it->setPosition(t);
}

void FBXMesh::setRotation(glm::mat4 t)
{
	for (auto& it : m_meshes)
		it->setRotation(t);
}

void FBXMesh::setScale(glm::mat4 t)
{
	for (auto& it : m_meshes)
		it->setScale(t);
}

glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& aiMat)
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

ostream& operator<<(ostream& os, const glm::vec2& v)
{
	return os << v.x << " " << v.y;
}

ostream& operator<<(ostream& os, const glm::vec3& v)
{
	return os << v.x << " " << v.y << " " << v.z;
}

ostream& operator<<(ostream& os, const glm::vec4& v)
{
	return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}

ostream& operator<<(ostream& os, const glm::mat3& m)
{
	return os << "\n" << m[0] << "\n" << m[1] << "\n" << m[2];
}

#include <iomanip> 
ostream& operator<<(ostream& os, const glm::mat4& m)
{
	return os << fixed << setprecision(1)
		<< m[0][0] << " " << m[1][0] << " " << m[2][0] << " " << m[3][0] << "\n"
		<< m[0][1] << " " << m[1][1] << " " << m[2][1] << " " << m[3][1] << "\n"
		<< m[0][2] << " " << m[1][2] << " " << m[2][2] << " " << m[3][2] << "\n"
		<< m[0][3] << " " << m[1][3] << " " << m[2][3] << " " << m[3][3];
}

ParticleMesh::ParticleMesh(const vector<info::VertexLayout>& layouts)
{
	m_buffer = make_unique<VertexBuffer>();
	m_buffer->createBuffers(layouts);
}

ParticleMesh::~ParticleMesh()
{
}

void ParticleMesh::updateBuffer(vector<info::VertexLayout> layouts)
{
	m_buffer->updateBuffer(layouts);
}

void ParticleMesh::drawInstance(const glm::mat4& P, const glm::mat4& V)
{
	m_buffer->bind();
		
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, GLsizei(m_buffer->getLayouts().size()));
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

	m_buffer->unbind();
}
