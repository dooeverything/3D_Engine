#include "Terrain.h"

#include "MapManager.h"
#include "Shader.h"
#include "ShaderManager.h"

Terrain::Terrain(float res) : Object("Terrain")
{
	cout << "*************************Terrain Constructor*************************" << endl;

	m_res = 64.0;
	m_width = 10.0f;
	m_height = 10.0f;

	m_noise_scale = 1;
	m_octaves = 8;
	m_frequency = 0.02f;

	m_is_edit = false;
	m_brush_size = 1.0f;
	m_strength = 0.05f;

	m_hit = glm::vec3(-1.0f);

	createVertex();

	vector<string> shader_paths = { "assets/shaders/Terrain.vert",
								   "assets/shaders/Terrain.frag",
								   "assets/shaders/Terrain.tcs",
								   "assets/shaders/Terrain.tes" };

	ShaderManager::createShader("Terrain", shader_paths);


	m_button_plus = make_unique<ImGuiButton>("assets/textures/TerrainPlus.png", "plus");
	m_button_minus = make_unique<ImGuiButton>("assets/textures/TerrainMinus.png", "minus");

	cout << "*************************Terrain Constructed!*************************" << endl;
	cout << endl;
}

Terrain::~Terrain()
{
}

int Terrain::getIndex(const glm::vec2& uv)
{
	return uv.x + m_res * uv.y;
}

void Terrain::createVertex()
{
	vector<info::VertexLayout> layouts;
	info::VertexLayout layout;
	m_vertices = vector<shared_ptr<TerrainVertex>>(m_res * m_res);
	for (float x = 0.0f; x < m_res-1; ++x)
	{
		for (float z = 0.0f; z < m_res-1; ++z)
		{
			layout.position.x = -m_width / 2.0f + m_width * x / m_res;
			layout.position.y = 0.0f;
			layout.position.z = -m_height / 2.0f + m_height * z / m_res;
			layout.texCoord.x = x / m_res;
			layout.texCoord.y = z / m_res;
			glm::vec3 p1 = layout.position;
			glm::vec2 uv1 = glm::vec2(x, z);
			layouts.push_back(layout);

			layout.position.x = -m_width / 2.0f + m_width * (x + 1) / m_res;
			layout.position.y = 0.0f;
			layout.position.z = -m_height / 2.0f + m_height * z / m_res;
			layout.texCoord.x = (x + 1) / m_res;
			layout.texCoord.y = z / m_res;
			glm::vec3 p2 = layout.position;
			glm::vec2 uv2 = glm::vec2(x+1, z);
			layouts.push_back(layout);

			layout.position.x = -m_width / 2.0f + m_width * x / m_res;
			layout.position.y = 0.0f;
			layout.position.z = -m_height / 2.0f + m_height * (z + 1) / m_res;
			layout.texCoord.x = x / m_res;
			layout.texCoord.y = (z + 1) / m_res;
			glm::vec3 p3 = layout.position;
			glm::vec2 uv3 = glm::vec2(x, z+1);
			layouts.push_back(layout);

			layout.position.x = -m_width / 2.0f + m_width * (x + 1) / m_res;
			layout.position.y = 0.0f;
			layout.position.z = -m_height / 2.0f + m_height * (z + 1) / m_res;
			layout.texCoord.x = (x + 1) / m_res;
			layout.texCoord.y = (z + 1) / m_res;
			glm::vec3 p4 = layout.position;
			glm::vec2 uv4 = glm::vec2(x+1, z+1);
			layouts.push_back(layout);

			shared_ptr<Tri> t1 = make_shared<Tri>(p1, p2, p4, uv1, uv2, uv4);
			shared_ptr<Tri> t2 = make_shared<Tri>(p1, p3, p4, uv1, uv3, uv4);
			m_trimeshes.push_back(t1);
			m_trimeshes.push_back(t2);

			shared_ptr<TerrainVertex> tv1 = make_shared<TerrainVertex>(p1, glm::vec3(1.0));
			if(m_vertices[getIndex(uv1)] == nullptr)
				m_vertices[getIndex(uv1)] = tv1;

			shared_ptr<TerrainVertex> tv2 = make_shared<TerrainVertex>(p2, glm::vec3(1.0));
			if(m_vertices[getIndex(uv2)] == nullptr)
				m_vertices[getIndex(uv2)] = tv2;

			shared_ptr<TerrainVertex> tv3 = make_shared<TerrainVertex>(p3, glm::vec3(1.0));
			if(m_vertices[getIndex(uv3)] == nullptr)
				m_vertices[getIndex(uv3)] = tv3;

			shared_ptr<TerrainVertex> tv4 = make_shared<TerrainVertex>(p4, glm::vec3(1.0));
			if(m_vertices[getIndex(uv4)] == nullptr)
				m_vertices[getIndex(uv4)] = tv4;
		}
	}

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	// color
	vector<glm::vec3> colors(layouts.size(), glm::vec3(1.0f));

	shared_ptr<Mesh> mesh = make_shared<Mesh>("Terrain");
	mesh->setupBuffer(layouts);
	addMesh(mesh);
}

void Terrain::updateVertex(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down)
{
	m_hit = glm::vec3(-1000.0f);
	if (!m_is_edit) return;

	float t = 0.0f;

	std::sort(m_trimeshes.begin(), m_trimeshes.end(),
		[ray_pos](const shared_ptr<Tri>& lhs, const shared_ptr<Tri>& rhs)
		{
			float d1 = glm::length(ray_pos - (lhs->getClosest(ray_pos)));
			float d2 = glm::length(ray_pos - (rhs->getClosest(ray_pos)));
			return d1 < d2;
		});

	float gap_w = m_width / m_res / 16.0f;
	float gap_h = m_height / m_res / 16.0f;
	int index = -1;
	vector<glm::vec3> offset = {{-gap_w, 0.0f, -gap_h}, 
								{-gap_w, 0.0f, 0.0f},
								{0.0f, 0.0f, -gap_h},
								{0.0f, 0.0f, 0.0f},
								{gap_w, 0.0f, 0.0f},
								{0.0f, 0.0f, gap_h},
								{gap_w, 0.0f, gap_h} };

	for (int i = 0; i < m_trimeshes.size(); ++i)
	{
		for (int j = 0; j < offset.size(); ++j)
		{
			if (m_trimeshes[i]->intersectWithRay(ray_dir+offset[j], ray_pos, t))
			{
				m_hit = ray_pos + ray_dir * t;
				//m_hit = hit_pos;
				index = i;
				break;
			}		
		}
	}

	if (index != -1)
	{
		glm::vec2 uv1 = m_trimeshes[index]->getUV1();
		glm::vec2 uv2 = m_trimeshes[index]->getUV2();
		glm::vec2 uv3 = m_trimeshes[index]->getUV3();

		glm::vec3 v1 = m_trimeshes[index]->getV1();
		glm::vec3 v2 = m_trimeshes[index]->getV2();
		glm::vec3 v3 = m_trimeshes[index]->getV3();
		
		glm::vec3 center = (v1 + v2 + v3) / 3.0f;
		
		for (int i = -1; i <= 1; ++i)
		{
			for (int j = -1; j <= 1; ++j)
			{
				glm::vec2 near1 = uv1 + glm::vec2(i, j);
				int idx1 = getIndex(near1);
				if (idx1 >= 0 && idx1 < m_res * m_res)
				{
					float r = glm::length(center - m_vertices[idx1]->pos);
					if (r <= m_brush_size && mouse_down)
					{
						m_vertices[idx1]->pos.y += (1.0 - r) * m_strength;
					}
				}

				glm::vec2 near2 = uv2 + glm::vec2(i, j);
				int idx2 = getIndex(near2);
				if (idx2 >= 0 && idx2 < m_res * m_res)
				{
					float r = glm::length(center - m_vertices[idx2]->pos);
					if (r <= m_brush_size && mouse_down)
					{
						m_vertices[idx2]->pos.y += (1.0 - r) * m_strength;
					}
				}
				
				glm::vec2 near3 = uv3 + glm::vec2(i, j);
				int idx3 = getIndex(near3);
				if (idx3 >= 0 && idx3 < m_res * m_res)
				{
					float r = glm::length(center - m_vertices[idx3]->pos);
					if (r <= m_brush_size && mouse_down)
					{
						m_vertices[idx3]->pos.y += (1.0 - r) * m_strength;
					}
				}
			}
		}
	}

	vector<info::VertexLayout> layouts = getVertices();
	for (int i = 0; i < layouts.size(); ++i)
	{
		glm::vec2 uv = layouts[i].texCoord;
		int idx = getIndex(uv*m_res);
		if (idx >= 0 && idx < m_res * m_res)
		{
			layouts[i].position = m_vertices[idx]->pos;
		}
	}

	updateBuffer(layouts);
	computeBBox();

	for (int i = 0; i < m_trimeshes.size(); ++i)
	{
		glm::vec2 uv1 = m_trimeshes[i]->getUV1();
		glm::vec2 uv2 = m_trimeshes[i]->getUV2();
		glm::vec2 uv3 = m_trimeshes[i]->getUV3();

		glm::vec3 a = m_vertices[getIndex(uv1)]->pos;
		glm::vec3 b = m_vertices[getIndex(uv2)]->pos;
		glm::vec3 c = m_vertices[getIndex(uv3)]->pos;

		m_trimeshes[i]->updateVertices(a, b, c);
	}
}

void Terrain::draw(const glm::mat4& P,
	const glm::mat4& V,
	const glm::vec3& view_pos,
	const Light& light)
{
	shared_ptr<Shader> shader = ShaderManager::getShader("Terrain");

	shader->load();
	glm::mat4 shadow_P = MapManager::getManager()->getShadowMapProj();
	glm::mat4 shadow_V = MapManager::getManager()->getShadowMapView();
	glm::mat4 shadow_proj = (shadow_P * shadow_V);
	shader->setMat4("light_matrix", shadow_proj);
	shader->setVec3("view_pos", view_pos);
	shader->setLight(light);
	shader->setVec3("hit_pos", m_hit);
	shader->setFloat("brush_size", m_brush_size);

	shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	MapManager::getManager()->bindShadowMap();

	drawTerrain(P, V, *shader, m_res);
}

void Terrain::renderExtraProperty()
{
	if (ImGui::CollapsingHeader("Terrain Edit"))
	{
		m_is_edit = false;

		static ImGuiTableFlags flags = ImGuiTableFlags_RowBg;
		ImVec2 cell_padding(0.0f, 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);
		ImGui::BeginTable("Terrain", 2);
		ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Brush Size");
		ImGui::TableNextColumn();
		string id_size = "##size";
		ImGui::SliderFloat(id_size.c_str(), &m_brush_size, 0.0f, 10.0f, "%.1f", 0);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Strength");
		ImGui::TableNextColumn();
		id_size = "##strength";
		ImGui::SliderFloat(id_size.c_str(), &m_strength, 0.0f, 5.0f, "%.1f", 0);

		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();

		ImGui::Text("Terrain Editor: ");

		//ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();

		if (m_button_plus->draw())
		{
			m_is_edit = true;
		}

		ImGui::SameLine();

		if (m_button_minus->draw())
		{
			m_is_edit = true;
		}
	
		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
	else
	{
		//t->setIsEdit(false);
		m_is_edit = false;
	}
}



