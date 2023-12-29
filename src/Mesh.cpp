#include "Mesh.h"
#include <fstream>
#include <sstream>

#include "Texture.h"
#include "Material.h"

Mesh::Mesh(string name) :
	m_name(name), m_buffer(make_unique<VertexBuffer>()),
	m_textures({}), m_material(make_unique<Material>())
{
	cout << "Create " << name << endl;
}

// Call from FBXMesh
Mesh::Mesh(string name, shared_ptr<VertexBuffer> buffer, 
		   vector<shared_ptr<Texture>> textures, 
		   shared_ptr<Material> material) :
	m_name(name),
	m_textures(textures), m_buffer(buffer),
	m_material(material)
{
	cout << "Create " << name << endl;
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

void Mesh::drawTerrain(
	const glm::mat4& P, const glm::mat4& V, const glm::mat4& M,
	Shader& shader, float res)
{
	//glm::mat4 M = m_transform_pos * m_transform_rot * m_transform_scale;
	shader.load();
	shader.setMaterial(*m_material);
	shader.setPVM(P, V, M);
	
	if (m_material->getTexture() != nullptr)
	{
		shader.setInt("has_texture", 1);
		shader.setInt("texture_map", 1);
		glActiveTexture(GL_TEXTURE0);
		m_material->getTexture()->setActive();
	}
	else
	{
		shader.setInt("has_texture", 0);
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_buffer->bind();
	glDrawArrays(GL_PATCHES, 0, 4 * res * res);
	m_buffer->unbind();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Mesh::draw(
	const glm::mat4& P, const glm::mat4& V, const glm::mat4& M,
	Shader& shader, bool terrain)
{
	unsigned int color_index = 1;
	unsigned int specular_index = 1;
	unsigned int normal_index = 1;
	unsigned int height_index = 1;

	glm::mat4 adjust = glm::mat4(1.0f);
	shader.load();
	shader.setMat4("adjust", adjust);
	shader.setMaterial(*m_material);

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

void Mesh::computeBBox(const glm::mat4& M)
{
	//cout << "Computer Bounding box " << m_name << endl;
	glm::vec3 b_min;
	glm::vec3 b_max;

	m_buffer->getBBoxBoundary(M, b_min, b_max);

	m_bbox = make_shared<BoundingBox>();
	m_bbox->setMin(b_min);
	m_bbox->setMax(b_max);
}
//void Mesh::drawLowQuality(Shader& shader)
//{
//	shader.load();
//	shader.setMaterial(*m_material);
//	glm::mat4 M = m_transform_pos * m_transform_rot * m_transform_scale;
//	shader.setMat4("model", M);
//	draw();
//}

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

