#include "Mesh.h"
#include <fstream>
#include <sstream>

#include "Texture.h"
#include "Material.h"
#include "Shader.h"
#include "Geometry.h"

Mesh::Mesh(string name) :
	m_name(name), m_buffer(make_unique<VertexBuffer>()),
	m_material(make_unique<Material>())
{
	cout << "Create " << name << endl;
}

// Call from FBXMesh
Mesh::Mesh(string name, shared_ptr<VertexBuffer> buffer, 
		   shared_ptr<Material> material) :
	m_name(name), m_buffer(buffer), m_material(material)
{
	cout << "Create " << name << endl;
}

void Mesh::draw(const Shader& shader)
{
	m_material->loadMaterialToShader(shader);

	if (m_buffer->getSizeOfIndices() > 1)
		draw();
	else
		drawArrays();

	// Set everything back to default texture
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::draw()
{
	m_buffer->bind();
	glDrawElements(GL_TRIANGLES, m_buffer->getSizeOfIndices(), GL_UNSIGNED_INT, nullptr);
	m_buffer->unbind();
}

void Mesh::drawArrays()
{
	m_buffer->bind();
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(m_buffer->getLayouts().size()));
	m_buffer->unbind();
}

void Mesh::drawInstance()
{
	m_buffer->bind();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawElementsInstanced(GL_POINTS, m_buffer->getSizeOfIndices(), GL_UNSIGNED_INT, 0, m_buffer->getSizeOfInstance());
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	m_buffer->unbind();
}

void Mesh::drawTerrain(const Shader& shader, float res)
{
	m_material->loadMaterialToShader(shader);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_buffer->bind();
	glDrawArrays(GL_PATCHES, 0, 4 * res * res);
	m_buffer->unbind();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::renderProperty(Sphere& preview_object, const FrameBuffer& preview_fb)
{
	m_material->renderProperty(preview_object, preview_fb);
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

void ParticleMesh::drawInstance()
{
	m_buffer->bind();
		
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, GLsizei(m_buffer->getLayouts().size()));
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

	m_buffer->unbind();
}

