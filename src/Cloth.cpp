#include "Cloth.h"
#include <cmath>

Cloth::Cloth()
{
	m_name = "Cloth";
	m_simulate = false;
	m_scale = 0.5f;

	t = 0.02f;
	n_sub_steps = 3;
	t_sub = t / n_sub_steps;

	string mesh_path = "assets/models/Cloth.fbx";
	m_mesh = make_shared<FBXMesh>(mesh_path);
	m_property[2] = glm::vec3(m_scale);
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::scale(t, m_property[2]);
	m_mesh->setScale(t);
	m_mesh->processMesh();
	
	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();
	
	initParticles();
}

Cloth::~Cloth()
{
}

void Cloth::initParticles()
{
	const vector<info::VertexLayout>& temp = m_mesh->getBuffer().getLayouts();

	glm::vec3 diff = temp[0].position - temp[1].position;
	m_rest = glm::length(diff);

	glm::vec3 size_cloth = m_mesh->getSize() * (1.0f / m_rest);
	m_width = size_cloth.x <= 0.0001f ? 0 : size_cloth.x;
	m_height = size_cloth.y <= 0.0001f ? 0 : size_cloth.y;
	m_depth = size_cloth.z <= 0.0001f ? 0 : size_cloth.z;
	m_particles.resize( size_t((m_width+1) * (m_height+1) * (m_depth+1)) );

	for (int i = 0; i < m_particles.size(); ++i)
	{
		m_particles[i] = nullptr;
	}

	vector<unsigned int> indices = m_mesh->getBuffer().getIndices();

	cout << endl;
	cout << "*************************Cloth Information**************************" << endl;
	cout << "Cloth size : " << size_cloth << " rest distance : " << m_rest << endl;
	cout << "Width : " << m_width << endl;
	cout << "Height : " << m_height << endl;
	cout << "Depth : " << m_depth << endl;
	cout << "Size of layout : " << temp.size() << endl;
	cout << "Size of indices : " << indices.size() << endl;
	cout << "********************************end*********************************" << endl;
	cout << endl;

	m_hash.reserve(info::HASH_SIZE);

	for (int i = 0; i < temp.size(); ++i)
	{
		glm::ivec3 grid_pos = getGridPos(temp[i].position);
		uint grid_index = getIndex(grid_pos);		
		if (m_particles[grid_index] == nullptr)
		{
			shared_ptr<ClothParticle> p = make_shared<ClothParticle>(temp[i].position);
			
			if (grid_index == 0 || grid_index == 16 )
			{
				p->m_pinned = true;
			}

			m_particles[grid_index] = p;
		}

		m_particles[grid_index]->m_ids.push_back(i);
	}
}

void Cloth::buildHash(vector<shared_ptr<ClothParticle>>& predict)
{
	for (int i = 0; i < m_hash.size(); ++i)
	{
		m_hash[i] = nullptr;
	}

	for (int i = 0; i < predict.size(); ++i)
	{
		ClothParticle* p = predict[i].get();
		glm::ivec3 grid_pos = getGridPos(p->m_position);
		uint hash_index = getHashIndex(grid_pos);
		
		if (m_hash[hash_index] == nullptr)
		{
			p->m_next = nullptr;
			m_hash[hash_index] = p;
		}
		else
		{
			p->m_next = m_hash[hash_index];
			m_hash[hash_index] = p;
		}
	}
}

uint Cloth::getIndex(glm::ivec3& pos)
{
	return uint(pos.x + (m_height + 1) * (pos.y + (m_depth + 1) * pos.z));
}

glm::ivec3 Cloth::getGridPos(glm::vec3 pos)
{
	return glm::ivec3(pos * (1.0f / m_rest));
}

uint Cloth::getHashIndex(glm::ivec3& pos)
{
	return ((uint)(pos.x * 73856093) ^
			(uint)(pos.y * 19349663) ^
			(uint)(pos.z * 83492791)) % info::HASH_SIZE;
}

void Cloth::update()
{
	vector<info::VertexLayout> update_layouts = m_mesh->getBuffer().getLayouts();

	for (int i = 0; i < update_layouts.size(); ++i)
	{
		update_layouts[i].normal = glm::vec3(0.0f);
	}

	vector<shared_ptr<ClothParticle>> predict(m_particles.size());
	vector<ClothParticle*> predict2(update_layouts.size(), nullptr);

	for (int i = 0; i < m_particles.size(); ++i)
	{
		ClothParticle* pi = m_particles[i].get();
		pi->m_velocity += pi->m_gravity * t_sub;
		glm::vec3 predict_pos = pi->m_position + pi->m_velocity * t_sub;
		predict[i] = make_shared<ClothParticle>(predict_pos);
		predict[i]->m_ids = pi->m_ids;

		if (pi->m_pinned)
		{
			predict[i]->m_mass = 0.0f;
		}

		for (int j = 0; j < pi->m_ids.size(); ++j)
		{
			predict2[pi->m_ids[j]] = predict[i].get();
		}
	}

	buildHash(predict);
	
	for (int sub = 0; sub < n_sub_steps; ++sub)
	{
		// Update predict position
		for (int i = 0; i < m_particles.size(); ++i)
		{
			ClothParticle* pi = m_particles[i].get();
			pi->m_velocity += pi->m_gravity * t_sub;
			glm::vec3 predict_pos = pi->m_position + pi->m_velocity * t_sub;
			predict[i]->m_position = predict_pos;
		}

 		// Compute Constraints
		for (int iter = 0; iter < 3; ++iter)
		{
			for (int i = 0; i < predict.size(); ++i)
			{
				updateStretch(i, predict);
			}

			for (int i = 0; i < predict2.size()-3; i +=4)
			{
				updateBending(i, 0.0f, predict2);
			}

			updateCollision(predict);
		}
		
		// Update Velocity & Position
		for (int i = 0; i < m_particles.size(); ++i)
		{
			ClothParticle* p = m_particles[i].get();
			
			if (p->m_pinned)
			{
				p->m_velocity = glm::vec3(0.0f);
			}
			else
			{
				p->m_velocity  = (predict[i]->m_position - p->m_position) / t_sub * (1.0f - 0.25f*t_sub);
				p->m_position  = predict[i]->m_position;			
			}
		}

		// Air resistance
		for (int i = 0; i < m_particles.size(); ++i)
		{
			m_particles[i]->m_velocity *= 0.998f;
		}
	}

	// Update Normal
	vector<unsigned int> indices = m_mesh->getBuffer().getIndices();
	for (int i = 0; i < indices.size()-2; i+=3)
	{		
		int id1 = indices[i];
		int id2 = indices[i+1];
		int id3 = indices[i+2];

		glm::vec3 p1 = predict2[id1]->m_position;
		glm::vec3 p2 = predict2[id2]->m_position;
		glm::vec3 p3 = predict2[id3]->m_position;

		glm::vec3 n = -glm::normalize(glm::cross(p2 - p1, p3 - p1));
		update_layouts[id1].normal = n;
		update_layouts[id2].normal = n;
		update_layouts[id3].normal = n;
	}

	// Update positions
	for (int i = 0; i < m_particles.size(); ++i)
	{
		ClothParticle* p = m_particles.at(i).get();
		for (int i = 0; i < p->m_ids.size(); ++i)
		{
			update_layouts[p->m_ids[i]].position = p->m_position; // *m_scale;
		}
	}

	m_mesh->updateBuffer(update_layouts);
}

void Cloth::updateStretch(int index, vector<shared_ptr<ClothParticle>>& predict)
{
	//uint index1 = getIndex(grid_pos);
	glm::vec3 p1 = predict[index]->m_position;
	float w1 = predict[index]->m_mass;

	int offset = int(max(m_width, max(m_height, m_depth))) + 1;
	glm::vec3 sum = glm::vec3(0.0f);

	if ( (index+1) % offset != 0 && (index+1) < predict.size())
	{
		// Right
		uint index2 = index + 1;
		glm::vec3 p2 = predict[index2]->m_position;
		float w2 = predict[index2]->m_mass;

		glm::vec3 diff = p1 - p2;
		float dist = glm::length(diff);

		if (dist > m_rest && w1 + w2 > 0.0f)
		{
			float lamda = (dist - m_rest) / (w1 + w2);
			glm::vec3 gradient = diff / dist;
			predict[index]->m_position  -= w1 * lamda * gradient;
			predict[index2]->m_position += w2 * lamda * gradient;
		}
	}

	if (index + offset < predict.size())
	{
		// Bottom
		uint index2 = index + int(offset);
		glm::vec3 p2 = predict[index2]->m_position;
		float w2 = predict[index2]->m_mass;

		glm::vec3 diff = p1 - p2;
		float dist = glm::length(diff);

		if (dist > m_rest && w1 + w2 > 0.0f)
		{
			float lamda = (dist - m_rest) / (w1 + w2);
			glm::vec3 gradient = diff / dist;
			predict[index]->m_position  -= w1 * lamda * gradient;
			predict[index2]->m_position += w2 * lamda * gradient;
		}
	}

	if ((index + 1) % offset != 0 && (index + offset) < predict.size())
	{
		// Bottom Right
		uint index2 = index + int(offset) + 1;
		glm::vec3 p2 = predict[index2]->m_position;
		float w2 = predict[index2]->m_mass;

		glm::vec3 diff = p1 - p2;
		float dist = glm::length(diff);

		float seperation_diagonal = glm::length(2 * m_rest);
		if (dist > seperation_diagonal && w1 + w2 > 0.0f)
		{
			float lamda = (dist - seperation_diagonal) / (w1 + w2);
			glm::vec3 gradient = diff / dist;
			predict[index]->m_position  -= w1 * lamda * gradient;
			predict[index2]->m_position += w2 * lamda * gradient;
		}
	}
}

void Cloth::updateBending(int index, float rest_angle, vector<ClothParticle*>& predict)
{
	glm::vec3 p1 = predict[index]->m_position;
	glm::vec3 p2 = predict[index+2]->m_position;
	glm::vec3 p3 = predict[index+3]->m_position;
	glm::vec3 p4 = predict[index+1]->m_position;

	float m1 = predict[index]->m_mass;
	float m2 = predict[index+2]->m_mass;
	float m3 = predict[index+3]->m_mass;
	float m4 = predict[index+1]->m_mass;

	glm::vec3 n1 = glm::normalize(glm::cross(p2 - p1, p3 - p1));
	glm::vec3 n2 = glm::normalize(glm::cross(p2 - p1, p4 - p1));
	
	//n1 /= glm::length2(n1);
	//n2 /= glm::length2(n2);

	float d = glm::clamp(glm::dot(n1, n2), 0.0f, 1.0f);
	float angle = acos(d);

	if (angle < 0.00001f || isnan(angle)) return;

	glm::vec3 u3 = glm::cross(p2, n2) + glm::cross(p2, n1) * d / glm::length(glm::cross(p2, p3));
	glm::vec3 u4 = glm::cross(p2, n1) + glm::cross(p2, n2) * d / glm::length(glm::cross(p2, p4));
	glm::vec3 u2 = -glm::cross(p3, n2) + glm::cross(p3, n1) * d / glm::length(glm::cross(p2, p3))
					-glm::cross(p4, n1) + glm::cross(p4, n2) * d / glm::length(glm::cross(p2, p4));
	glm::vec3 u1 = -u2-u3-u4;

	float lamda = 10.0f/ (t_sub*t_sub) + m1 * (glm::dot(u1, u1) + m2 * glm::dot(u2, u2) + 
				m3 * glm::dot(u3, u3) + m4 * glm::dot(u4, u4));
	
	if (lamda != 0.0f)
	{
		lamda = sqrt(1.0f - d * d) * (angle - rest_angle) / lamda;

		predict[index]->m_position	  -= m1 * u1 * lamda;
		predict[index + 1]->m_position -= m2 * u2 * lamda;
		predict[index + 2]->m_position -= m3 * u3 * lamda;
		predict[index + 3]->m_position -= m4 * u4 * lamda;
	}
}

void Cloth::updateCollision(vector<shared_ptr<ClothParticle>>& predict)
{
	for (int i = 0; i < predict.size(); ++i)
	{
		ClothParticle* curr = predict[i].get();
		glm::vec3 p1 = predict[i]->m_position;
		float w1 = predict[i]->m_mass;
		glm::ivec3 p1_grid = getGridPos(p1);

		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				for (int z = -1; z <= 1; z++)
				{
					glm::ivec3 near_pos = p1_grid + glm::ivec3(x, y, z);
					uint index = getHashIndex(near_pos);
					ClothParticle* neighbor = m_hash[index];

					while (neighbor)
					{
						glm::vec3 p2 = neighbor->m_position;
						float w2 = neighbor->m_mass;
						
						glm::vec3 diff = p1 - p2;
						float dist = glm::length(p1 - p2);

						if (dist < m_rest && w1 + w2 > 0.0f && curr != neighbor)
						{
							glm::vec3 gradient = diff / (dist + 0.000001f);
							float lamda = (dist - m_rest) / (w1 + w2);
							predict[i]->m_position -= w1 * lamda * gradient;
							neighbor->m_position   += w2 * lamda * gradient;
						}

						neighbor = neighbor->m_next;
					}
				}
			}
		}
	}

}


