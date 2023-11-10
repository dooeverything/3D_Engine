// Create tetradralized mesh from surface mesh
// Reference : https://github.com/crawforddoran/quartet
// Copyright (c) 2014, crawforddoran <>

#include "SoftBodySolver.h"

SoftBodySolver::SoftBodySolver(Mesh* mesh) :
	m_simulate(false), m_og_layouts(mesh->getBuffer().getLayouts())
{
	cout << endl;
	
	cout << "********************Add SoftBodySolver********************" << endl;

	m_dx = 0.1;
	m_mesh = mesh;
	m_layouts = mesh->getBuffer().getLayouts();

	m_hash_size = m_layouts.size() * 2;
	m_hash_start.resize(m_hash_size + 1);
	m_hash_ids.resize(m_layouts.size());

	buildHash();

	TetMesh tet_mesh;
	getTet(tet_mesh);
	getTetVertices(tet_mesh);
	//getEdgeVertices(tet_mesh);
	
	getTetIds();
	//getEdgeIds();

	m_simulate = false;
	t = 0.06f;
	n_sub_steps = 2;
	t_sub = t / n_sub_steps;

	cout << "********************end********************\n" << endl;
}


SoftBodySolver::~SoftBodySolver()
{
	cout << "Delete softbodysolver" << endl;
}

void SoftBodySolver::getTet(TetMesh& tet_mesh)
{
	vector<Vec3f> surf_x;
	vector<Vec3i> surf_f;

	for (int i = 0; i < m_layouts.size(); ++i)
	{
		Vec3f p(m_layouts[i].position.x, m_layouts[i].position.y, m_layouts[i].position.z);
		surf_x.push_back(p);
	}

	vector<uint> indices = m_mesh->getBuffer().getIndices();
	for (int i = 0; i < indices.size() - 2; i += 3)
	{
		Vec3i index(indices[i], indices[i + 1], indices[i + 2]);
		surf_f.push_back(index);
	}

	Vec3f xmin(m_mesh->getBox().getMin().x, m_mesh->getBox().getMin().y, m_mesh->getBox().getMin().z);
	Vec3f xmax(m_mesh->getBox().getMax().x, m_mesh->getBox().getMax().y, m_mesh->getBox().getMax().z);

	Vec3f origin = xmin - Vec3f(2 * m_dx);
	int ni = (int)std::ceil((xmax[0] - xmin[0]) / m_dx) + 5,
		nj = (int)std::ceil((xmax[1] - xmin[1]) / m_dx) + 5,
		nk = (int)std::ceil((xmax[2] - xmin[2]) / m_dx) + 5;

	SDF sdf(origin, m_dx, ni, nj, nk); // Initialize signed distance field.
	make_signed_distance(surf_f, surf_x, sdf);

	// Make tet mesh without features
	cout << "Making tet mesh" << endl;;
	make_tet_mesh(tet_mesh, sdf, false, false, false);
	cout << "Done tet mesh" << endl;
}

void SoftBodySolver::getTetVertices(const TetMesh& tet_mesh)
{
	vector<Vec3f> tet_vertices = tet_mesh.verts();
	for (int i = 0; i < tet_mesh.vSize(); ++i)
	{
		glm::vec3 pos;
		pos.x = tet_vertices[i][0];
		pos.y = tet_vertices[i][1];
		pos.z = tet_vertices[i][2];

		shared_ptr<SoftParticle> p = make_shared<SoftParticle>(pos);
		m_tets.push_back(p);
	}
	cout << "tet vertices size : " << m_tets.size() << endl;

	// Get rest volume for each tetradehral
	vector<Vec4i> tet_indices = tet_mesh.tets();
	for (int i = 0; i < tet_mesh.tSize(); ++i)
	{
		int idx0 = tet_indices[i][0];
		int idx1 = tet_indices[i][1];
		int idx2 = tet_indices[i][2];
		int idx3 = tet_indices[i][3];

		m_tet_indices.push_back(idx0);
		m_tet_indices.push_back(idx1);
		m_tet_indices.push_back(idx2);
		m_tet_indices.push_back(idx3);

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

void SoftBodySolver::getEdgeVertices(const TetMesh& tet_mesh)
{
	vector<Vec3f> boundary_verts;
	vector<Vec3i> boundary_indices;
	tet_mesh.getBoundary(boundary_verts, boundary_indices);

	for (int i = 0; i < boundary_verts.size(); ++i)
	{
		glm::vec3 pos;
		pos.x = boundary_verts[i][0];
		pos.y = boundary_verts[i][1];
		pos.z = boundary_verts[i][2];

		shared_ptr<SoftParticle> p = make_shared<SoftParticle>(pos);
		m_edges.push_back(p);
	}

	m_rest_d.resize(boundary_indices.size() * 3);
	cout << "edge indices size: " << m_rest_d.size() << endl;

	for (int i = 0; i < boundary_indices.size(); ++i)
	{
		uint idx0 = boundary_indices[i][0];
		uint idx1 = boundary_indices[i][1];
		uint idx2 = boundary_indices[i][2];

		m_edges_indices.push_back(idx0);
		m_edges_indices.push_back(idx1);
		m_edges_indices.push_back(idx2);

		glm::vec3 p0 = m_edges[idx0]->m_position;
		glm::vec3 p1 = m_edges[idx1]->m_position;
		glm::vec3 p2 = m_edges[idx2]->m_position;

		m_rest_d[i] = glm::length(p0 - p1);
		m_rest_d[i + 1] = glm::length(p1 - p2);
		m_rest_d[i + 2] = glm::length(p2 - p0);
	}
}

void SoftBodySolver::getTetIds()
{
	cout << "Get ids for tets" << endl;
	vector<int> test;
	for (int i = 0; i < m_tets.size(); ++i)
	{
		SoftParticle* pi = m_tets[i].get();
		glm::ivec3 grid_pos = getGridPos(pi->m_position);
		for (int x = -2; x <= 2; x += 1)
		{
			for (int y = -2; y <= 2; y += 1)
			{
				for (int z = -2; z <= 2; z += 1)
				{
					glm::ivec3 p1 = grid_pos + glm::ivec3(x, y, z);
					uint h = getHashIndex(p1);

					int start = m_hash_start[h];
					int end = m_hash_start[h + 1];
					for (int j = start; j < end; ++j)
					{
						glm::vec3 p2 = m_layouts[m_hash_ids[j]].position;

						float dist = glm::length(p2 - pi->m_position);
						if (dist < 0.12)
						{
							//cout << " vs p2: " << p2 << " with " << m_hash_ids[j] << endl;
							pi->m_indices.push_back(m_hash_ids[j]);
							test.push_back(m_hash_ids[j]);
						}
					}
				}
			}
		}
	}

	std::sort(test.begin(), test.end());
	vector<info::VertexLayout> new_layouts;

	cout << "Test" << endl;
	for (int i = 0; i < test.size(); ++i)
	{
		//cout << test[i] << " ";

		info::VertexLayout layout;
		layout.position = m_layouts[test[i]].position;
		new_layouts.push_back(layout);
	}
	cout << endl;

	m_tet_mesh = make_unique<Mesh>("tet");
	m_tet_mesh->getBuffer().createBuffers(new_layouts);
}

void SoftBodySolver::getEdgeIds()
{
	vector<int> test;
	cout << "Get ids for edges" << endl;
	for (int i = 0; i < m_edges.size(); ++i)
	{
		SoftParticle* pi = m_edges[i].get();
		glm::ivec3 grid_pos = getGridPos(pi->m_position);
		for (int x = -2; x <= 2; x += 1)
		{
			for (int y = -2; y <= 2; y += 1)
			{
				for (int z = -2; z <= 2; z += 1)
				{
					glm::ivec3 p1 = grid_pos + glm::ivec3(x, y, z);
					uint h = getHashIndex(p1);

					int start = m_hash_start[h];
					int end = m_hash_start[h + 1];
					for (int j = start; j < end; ++j)
					{
						glm::vec3 p2 = m_layouts[m_hash_ids[j]].position;

						float dist = glm::length(p2 - pi->m_position);
						if (dist < 0.12)
						{
							pi->m_indices.push_back(m_hash_ids[j]);
							test.push_back(m_hash_ids[j]);
						}
					}
				}
			}
		}
	}

	std::sort(test.begin(), test.end());
	vector<info::VertexLayout> new_layouts;

	cout << "Test" << endl;
	for (int i = 0; i < test.size(); ++i)
	{
		cout << test[i] << " ";

		info::VertexLayout layout;
		layout.position = m_layouts[test[i]].position;
		new_layouts.push_back(layout);
	}
	cout << endl;

	m_tet_mesh = make_unique<Mesh>("tet");
	m_tet_mesh->getBuffer().createBuffers(new_layouts);
}

glm::ivec3 SoftBodySolver::getGridPos(glm::vec3 pos)
{
	int x = int(pos.x / m_dx);
	int y = int(pos.y / m_dx);
	int z = int(pos.z / m_dx);
	return glm::ivec3(x, y, z);
}

uint SoftBodySolver::getHashIndex(glm::ivec3& pos)
{
	return ((uint)(pos.x * 92837111) ^
			(uint)(pos.y * 689287499) ^
			(uint)(pos.z * 283923481)) % m_hash_size;
}

void SoftBodySolver::simulate()
{
	if (m_simulate == false) return;

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
				p->m_velocity *= -0.5f;
				p->m_position.y = -5.0f;
			}
		}
	}
	
	for (int i = 0; i < m_tets.size(); ++i)
	{
		m_tets[i]->m_velocity *= 0.80f;
	}

	// update layouts
	for (int i = 0; i < m_tets.size(); ++i)
	{
		SoftParticle* p = m_tets[i].get();
		for (int j = 0; j < p->m_indices.size(); ++j)
		{
			int idx = p->m_indices[j];
			m_layouts[idx].position = p->m_position;
		}
	}

	m_mesh->getBuffer().updateBuffer(m_layouts);

	buildHash();
}

void SoftBodySolver::buildHash()
{
	for (int i = 0; i < m_hash_start.size(); ++i)
	{
		m_hash_start[i] = 0;
	}

	for (int i = 0; i < m_hash_ids.size(); ++i)
	{
		m_hash_ids[i] = 0;
	}

	for (int i = 0; i < m_layouts.size(); ++i)
	{
		glm::ivec3 grid_pos = getGridPos(m_layouts[i].position);
		uint hash_index = getHashIndex(grid_pos);
		m_hash_start[hash_index]++;
	}

	uint sum = 0;
	for (int i = 0; i < m_hash_start.size()-1; ++i)
	{
		sum += m_hash_start[i];
		m_hash_start[i] = sum;
	}
	m_hash_start[m_hash_size] = sum;

	for (int i = 0; i < m_layouts.size(); ++i)
	{
		glm::ivec3 grid_pos = getGridPos(m_layouts[i].position);
		uint hash_index = getHashIndex(grid_pos);
		m_hash_start[hash_index]--;
		m_hash_ids[m_hash_start[hash_index]] = i;
	}
}

void SoftBodySolver::computePredictEdges()
{
	for (int i = 0; i < m_edges.size(); ++i)
	{
		SoftParticle* p = m_edges[i].get();
		p->m_velocity += p->m_gravity * t_sub;
		p->m_position += p->m_velocity * t_sub;
	}
}

void SoftBodySolver::computePredictTets()
{
	for (int i = 0; i < m_tets.size(); ++i)
	{
		SoftParticle* p = m_tets[i].get();
		p->m_velocity += p->m_gravity * t_sub;
		p->m_position += p->m_velocity * t_sub;
	}
}

void SoftBodySolver::solveDistance(vector<glm::vec3>& predict)
{
	int edge_idx = 0;
	for (int i = 0; i < m_tet_indices.size() - 3; i += 4)
	{

		int idx0 = m_tet_indices[i];
		int idx1 = m_tet_indices[i + 1];
		int idx2 = m_tet_indices[i + 2];
		int idx3 = m_tet_indices[i + 3];
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

void SoftBodySolver::solveVolume(vector<glm::vec3>& predict)
{
	for (int i = 0; i < m_tet_indices.size() - 3; i += 4)
	{
		int idx0 = m_tet_indices[i];
		int idx1 = m_tet_indices[i + 1];
		int idx2 = m_tet_indices[i + 2];
		int idx3 = m_tet_indices[i + 3];

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

void SoftBodySolver::reset()
{
	//cout << "Reset" << endl;
	//for (int i = 0; i < m_layouts.size(); ++i)
	//{
	//	m_layouts[i].position = m_og_layouts[i].position;
	//}
	//m_mesh->getBuffer().updateBuffer(m_layouts);
}

void SoftBodySolver::renderProperty()
{
	//cout << "hello" << endl;
	bool expand_softbody = ImGui::TreeNode("SoftBodySolver");
	if (expand_softbody)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		ImVec2 cell_padding(0.0f, 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
		ImGui::BeginTable("Simulation", 2);
		{
			ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();

			static int clicked_simulate = 0;
			if (ImGui::Button("Click to simulate"))
			{
				clicked_simulate++;
			}

			if (clicked_simulate & 1)
			{
				ImGui::TableNextColumn();
				if (ImGui::Button("Start") && m_simulate == false)
				{
					m_simulate = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Stop") && m_simulate == true)
				{
					m_simulate = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("Reset") && m_simulate == true)
				{
					m_simulate = false;
					reset();
				}
			}		
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
		ImGui::TreePop();
	}
}
