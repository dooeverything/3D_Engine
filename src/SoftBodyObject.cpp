// Create tetradralized mesh from surface mesh
// Reference : https://github.com/crawforddoran/quartet
// Copyright (c) 2014, crawforddoran <>

#include "SoftBodyObject.h"

#include "Mesh.h"
#include "Particle.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "MapManager.h"

SoftBodyObject::SoftBodyObject(
	const vector<info::VertexLayout>& vertices,
	const vector<info::uint>& indices,
	const Transform& transform,
	const glm::vec3& b_min, const glm::vec3& b_max) : Object("SoftBody"), m_simulate(false), m_reset(false)
{
	cout << endl;
	cout << "********************Add SoftBodySolver********************" << endl;

	cout << "Box Min: " << glm::to_string(b_min) << endl;
	cout << "Box Max: " << glm::to_string(b_max) << endl;

	setTransform(transform);

	m_dx = 0.1;

	vector<glm::vec3> transformed_vertices = transformVertices(vertices);
	getTet(transformed_vertices, indices, b_min, b_max);

	m_simulate = false;
	t = 0.06f;
	n_sub_steps = 2;
	t_sub = t / n_sub_steps;

	cout << "********************end********************\n" << endl;
}

SoftBodyObject::~SoftBodyObject()
{
	cout << "Delete softbodysolver" << endl;
}

vector<glm::vec3> SoftBodyObject::transformVertices(
	const vector<info::VertexLayout>& vertices)
{
	vector<glm::vec3> new_positions;
	new_positions.resize(vertices.size());

	for (int i = 0; i < vertices.size(); ++i)
	{
		glm::vec4 p = getModelTransform() * glm::vec4(vertices.at(i).position, 1.0f);
		new_positions.at(i) = p;
	}

	return new_positions;
}

void SoftBodyObject::draw(
	const glm::mat4& P, 
	const glm::mat4& V, 
	const glm::vec3& view_pos, 
	const Light& light)
{
	simulate();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	Object::draw(P, V, view_pos, light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SoftBodyObject::getTet(
	const vector<glm::vec3>& vertices,
	const vector<info::uint>& indices,
	const glm::vec3& b_min,
	const glm::vec3& b_max)
{
	cout << "Get tet" << endl;
	vector<Vec3f> surf_x;
	vector<Vec3i> surf_f;

	for (int i = 0; i < vertices.size(); ++i)
	{
		Vec3f p(vertices[i].x, vertices[i].y, vertices[i].z);
		surf_x.push_back(p);
	}

	for (int i = 0; i < indices.size() - 2; i += 3)
	{
		Vec3i index(indices[i], indices[i + 1], indices[i + 2]);
		surf_f.push_back(index);
	}

	cout << "Surf vertices: " << surf_x.size() << endl;
	cout << "Surf Faces: " << surf_f.size() << endl;

	Vec3f xmin(b_min.x, b_min.y, b_min.z);
	Vec3f xmax(b_max.x, b_max.y, b_max.z);

	Vec3f origin = xmin - Vec3f(2 * m_dx);
	int ni = (int)std::ceil((xmax[0] - xmin[0]) / m_dx) + 5,
		nj = (int)std::ceil((xmax[1] - xmin[1]) / m_dx) + 5,
		nk = (int)std::ceil((xmax[2] - xmin[2]) / m_dx) + 5;

	SDF sdf(origin, m_dx, ni, nj, nk); // Initialize signed distance field.
	make_signed_distance(surf_f, surf_x, sdf);

	// Make tet mesh without features
	cout << "*Making tet mesh*" << endl;
	TetMesh tet_mesh;
	make_tet_mesh(tet_mesh, sdf, false, false, false);
	cout << "*Done tet mesh*" << endl;

	getTetVertices(tet_mesh);
}

void SoftBodyObject::getTetVertices(const TetMesh& tet_mesh)
{
	vector<Vec3f> tet_vertices = tet_mesh.verts();
	for (int i = 0; i < tet_mesh.vSize(); ++i)
	{
		glm::vec3 pos;
		pos.x = tet_vertices[i][0];
		pos.y = tet_vertices[i][1];
		pos.z = tet_vertices[i][2];

		shared_ptr<SoftParticle> p = make_shared<SoftParticle>(pos);
		m_tets.emplace_back(p);
	
		info::VertexLayout v;
		v.position = pos;
		m_tet_vertices.emplace_back(v);
		m_tet_vertices_og.emplace_back(v);
	}

	// Get rest volume for each tetradehral
	vector<Vec4i> tet_indices = tet_mesh.tets();
	for (int i = 0; i < tet_mesh.tSize(); ++i)
	{
		int idx0 = tet_indices[i][0];
		int idx1 = tet_indices[i][1];
		int idx2 = tet_indices[i][2];
		int idx3 = tet_indices[i][3];

		m_faces.push_back(idx0);
		m_faces.push_back(idx1);
		m_faces.push_back(idx2);
		m_faces.push_back(idx3);

		// 0, 2, 1
		m_tet_indices.push_back(idx0);
		m_tet_indices.push_back(idx2);
		m_tet_indices.push_back(idx1);

		// 2, 3, 1
		m_tet_indices.push_back(idx2);
		m_tet_indices.push_back(idx3);
		m_tet_indices.push_back(idx1);

		glm::vec3 p0 = m_tets[idx0]->m_position;
		glm::vec3 p1 = m_tets[idx1]->m_position;
		glm::vec3 p2 = m_tets[idx2]->m_position;
		glm::vec3 p3 = m_tets[idx3]->m_position;

		glm::vec3 e0 = p1 - p0;
		glm::vec3 e1 = p2 - p0;
		glm::vec3 e2 = p3 - p0;

		float v = glm::dot(glm::cross(e0, e1), e2) / 6.0f;
		m_rest_v.push_back(v);
	}

	cout << " -tet vertice size: " << m_tet_vertices.size() << ", tet indices size: " << m_tet_indices.size() << endl;
	
	if (m_tet_vertices.empty() || m_tet_indices.empty())
	{
		cout << "Something wrong with the tet mesh" << endl;
		assert(0);
	}
	
	shared_ptr<Mesh> mesh = make_unique<Mesh>("SoftBody");
	mesh->setupBuffer(m_tet_vertices, m_tet_indices);
	addMesh(mesh);

	// Get rest distance for each edge of each tetrahedral
	// 6 edges for each tetrahedral	
	for (int i = 0; i < tet_mesh.tSize(); ++i)
	{
		int idx0 = tet_indices[i][0];
		int idx1 = tet_indices[i][1];
		int idx2 = tet_indices[i][2];
		int idx3 = tet_indices[i][3];

		glm::vec3 p0 = m_tets[idx0]->m_position;
		glm::vec3 p1 = m_tets[idx1]->m_position;
		glm::vec3 p2 = m_tets[idx2]->m_position;
		glm::vec3 p3 = m_tets[idx3]->m_position;

		glm::vec3 e01 = p1 - p0;
		glm::vec3 e02 = p2 - p0;
		glm::vec3 e03 = p3 - p0;
		glm::vec3 e12 = p2 - p1;
		glm::vec3 e13 = p3 - p1;
		glm::vec3 e23 = p3 - p2;

		float d0 = glm::length(e01);
		float d1 = glm::length(e02);
		float d2 = glm::length(e03);
		float d3 = glm::length(e12);
		float d4 = glm::length(e13);
		float d5 = glm::length(e23);

		m_rest_d.push_back(d0);
		m_rest_d.push_back(d1);
		m_rest_d.push_back(d2);
		m_rest_d.push_back(d3);
		m_rest_d.push_back(d4);
		m_rest_d.push_back(d5);
	}

	cout << "Size of m_rest_v: " << m_rest_v.size() << " size of m_rest_d: " << m_rest_d.size() << endl;
}

void SoftBodyObject::simulate()
{
	if (m_simulate == false)
	{
		if (m_reset)
		{
			reset();
			cout << "Reset" << endl;
			m_reset = false;
		}
		return;
	}

	m_reset = true;

	vector<glm::vec3> predict2(m_tets.size());

	for (int sub = 0; sub < n_sub_steps; ++sub)
	{
		for (int i = 0; i < predict2.size(); ++i)
		{
			SoftParticle* p = m_tets[i].get();
			glm::vec3 v = p->m_velocity + p->m_gravity * t_sub;
			predict2[i] = p->m_position + v * t_sub;
		}

		for (int iter = 0; iter < 4; ++iter)
		{
			solveDistance(predict2);
			solveVolume(predict2);
		}

		for (int i = 0; i < m_tets.size(); ++i)
		{
			// Update positions
			SoftParticle* p = m_tets[i].get();
			if (p->m_pinned)
			{
				p->m_velocity = glm::vec3(0.0f);
			}
			else
			{
				p->m_velocity = (predict2[i] - p->m_position) / t_sub;
				p->m_position = predict2[i];
			}

			if (p->m_position.y < -5.0f)
			{
				p->m_velocity  *= -0.8f;
				p->m_position.y = -5.0f;
			}
		}
	}
	
	for (int i = 0; i < m_tets.size(); ++i)
	{
		m_tets[i]->m_velocity *= 0.50f;
	}

	// update layouts
	for (int i = 0; i < m_tets.size(); ++i)
	{
		SoftParticle* p = m_tets[i].get();
		m_tet_vertices[i].position = p->m_position;
	}

	updateBuffer(m_tet_vertices);
}

void SoftBodyObject::solveDistance(vector<glm::vec3>& predict)
{
	int edge_idx = 0;
	for (int i = 0; i < m_faces.size() - 3; i += 4)
	{
		int idx0 = m_faces[i];
		int idx1 = m_faces[i + 1];
		int idx2 = m_faces[i + 2];
		int idx3 = m_faces[i + 3];
		//cout << "solving distance at " << edge_idx << endl;

		glm::vec3 p0 = predict[idx0];
		glm::vec3 p1 = predict[idx1];
		glm::vec3 p2 = predict[idx2];
		glm::vec3 p3 = predict[idx3];

		if (glm::any(glm::isnan(p0)))
		{
			cout << "Nan detected p0 " << edge_idx << endl;
			assert(0);
		}

		if (glm::any(glm::isnan(p1)))
		{
			cout << "Nan detected p0" << endl;
			assert(0);
		}

		if (glm::any(glm::isnan(p2)))
		{
			cout << "Nan detected p0" << endl;
			assert(0);
		}
		
		if (glm::any(glm::isnan(p3)))
		{
			cout << "Nan detected p0" << endl;
			assert(0);
		}

		float w0 = 1.0f / m_tets[idx0]->m_mass;
		float w1 = 1.0f / m_tets[idx1]->m_mass;
		float w2 = 1.0f / m_tets[idx2]->m_mass;
		float w3 = 1.0f / m_tets[idx3]->m_mass;

		glm::vec3 e01 = p1 - p0;
		glm::vec3 e02 = p2 - p0;
		glm::vec3 e03 = p3 - p0;
		glm::vec3 e12 = p2 - p1;
		glm::vec3 e13 = p3 - p1;
		glm::vec3 e23 = p3 - p2;

		float d0 = glm::length(e01);
		float d1 = glm::length(e02);
		float d2 = glm::length(e03);
		float d3 = glm::length(e12);
		float d4 = glm::length(e13);
		float d5 = glm::length(e23);

		float a = 0.0f / (t_sub * t_sub);

		float lambda = (d0 - m_rest_d[edge_idx]) / (w0 + w1 + a);
		glm::vec3 n = e01 / d0;
		predict[idx0] += w0 * lambda * n;
		predict[idx1] -= w1 * lambda * n;

		lambda = (d1 - m_rest_d[edge_idx + 1]) / (w2 + w0 + a);
		n = e02 / d1;
		predict[idx0] += w0 * lambda * n;
		predict[idx2] -= w2 * lambda * n;

		lambda = (d2 - m_rest_d[edge_idx + 2]) / (w3 + w0 + a);
		n = e03 / d2;
		predict[idx0] += w0 * lambda * n;
		predict[idx3] -= w3 * lambda * n;

		lambda = (d3 - m_rest_d[edge_idx + 3]) / (w1 + w2 + a);
		n = e12 / d3;
		predict[idx1] += w1 * lambda * n;
		predict[idx2] -= w2 * lambda * n;

		lambda = (d4 - m_rest_d[edge_idx + 4]) / (w1 + w3 + a);
		n = e13 / d4;
		predict[idx1] += w1 * lambda * n;
		predict[idx3] -= w3 * lambda * n;

		lambda = (d5 - m_rest_d[edge_idx + 5]) / (w2 + w3 + a);
		n = e23 / d5;
		predict[idx2] += w2 * lambda * n;
		predict[idx3] -= w3 * lambda * n;

		edge_idx += 6;
	}
}

void SoftBodyObject::solveVolume(vector<glm::vec3>& predict)
{
	for (int i = 0; i < m_faces.size() - 3; i += 4)
	{
		int idx0 = m_faces[i];
		int idx1 = m_faces[i + 1];
		int idx2 = m_faces[i + 2];
		int idx3 = m_faces[i + 3];

		glm::vec3 p0 = predict[idx0];
		glm::vec3 p1 = predict[idx1];
		glm::vec3 p2 = predict[idx2];
		glm::vec3 p3 = predict[idx3];

		glm::vec3 e0 = p1 - p0;
		glm::vec3 e1 = p2 - p0;
		glm::vec3 e2 = p3 - p0;

		float v = glm::dot(glm::cross(e0, e1), e2) / 6.0f;

		glm::vec3 u1 = glm::cross(p3 - p1, p2 - p1);
		glm::vec3 u2 = glm::cross(p3 - p0, p2 - p0);
		glm::vec3 u3 = glm::cross(p3 - p0, p1 - p0);
		glm::vec3 u4 = glm::cross(p2 - p0, p1 - p0);

		float a = 10.0f / (t_sub * t_sub);
		float lambda = a + glm::dot(u1, u1) + glm::dot(u2, u2) + glm::dot(u3, u3) + glm::dot(u4, u4);
		if (lambda != 0.0f)
		{
			lambda = (v - m_rest_v[i/4]) / lambda;
			predict[idx0] -= u1 * lambda;
			predict[idx1] -= u2 * lambda;
			predict[idx2] -= u3 * lambda;
			predict[idx3] -= u4 * lambda;
		}
	}
}

void SoftBodyObject::reset()
{
	updateBuffer(m_tet_vertices_og);

	for (int i = 0; i < m_tets.size(); ++i)
	{
		m_tets.at(i)->m_position = m_tet_vertices_og.at(i).position;
	}
}
