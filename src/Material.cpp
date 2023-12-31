#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "FileDialog.h"
#include "Geometry.h"
#include "imgui-docking/imgui.h"
#include "Buffer.h"
#include "Light.h"

Material::Material() :
	m_fd(make_unique<FileDialog>()),
	m_textures_base({}), m_textures_specular({}),
	m_textures_normal({}), m_textures_height({})
{
	m_type = "";
	m_base_color = { 0.8f, 0.8f, 0.8f };
	m_metallic = 1.0f;
	m_roughness = 0.5f;
	m_ao = 1.0f;
}

Material::~Material()
{}

void Material::loadMaterialToShader(const Shader& shader)
{
	shader.load();
	if (!hasTexture())
	{
		shader.setInt("type", 0);
		shader.setMaterial(*this);
	}
	else
	{
		shader.setInt("type", 1);
		shader.setInt("texture_map", 4);
		glActiveTexture(GL_TEXTURE0 + 4);
		m_textures_base.at(0)->setActive();
	}
}

bool Material::hasTexture()
{
	if (m_textures_base.empty())
		return false;

	return true;
}

void Material::addTexture(const string& path)
{
	shared_ptr<Texture> t = make_shared<Texture>(path);
	t->loadTexture();

	if (m_textures_base.size() > 0)
	{
		m_textures_base.clear();
	}

	m_textures_base.emplace_back(t);
}

void Material::renderProperty(Sphere& preview_object, const FrameBuffer& preview_fb)
{
	if (!hasTexture())
	{
		ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_DisplayRGB;
		misc_flags |= ImGuiColorEditFlags_NoSidePreview;
		ImGui::Text("Color");
		ImGui::ColorPicker3("##Color", (float*)&m_base_color, misc_flags);

		static float slider_f = 0.5f;
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		ImVec2 cell_padding(-1.0f, 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);

		ImGui::BeginTable("PBR", 2);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();

		ImGui::Text("Metallic");
		ImGui::TableNextColumn();
		ImGui::SliderFloat("##Metallic", &m_metallic, 0.0f, 1.0f, "%.3f", flags);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::Text("Roughness");
		ImGui::TableNextColumn();
		ImGui::SliderFloat("##Roughness", &m_roughness, 0.0f, 1.0f, "%.3f", flags);

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
	else
	{
		shared_ptr<Shader> shader = ShaderManager::getShader("Default");

		glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
		glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 M = glm::mat4(1.0f);

		glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
		glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
		glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
		glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
		unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
		glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

		shader->load();
		shader->setVec3("view_pos", view_pos);
		shader->setLight(*light);
		shader->setPVM(P, V, M);

		shader->setInt("shadow_map", 0);
		glActiveTexture(GL_TEXTURE0);
		shader->setInt("irradiance_map", 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		shader->setInt("prefilter_map", 2);
		glActiveTexture(GL_TEXTURE0 + 2);
		shader->setInt("lut_map", 3);
		glActiveTexture(GL_TEXTURE0 + 3);

		shader->setInt("type", 1);
		shader->setInt("texture_map", 4);
		glActiveTexture(GL_TEXTURE0 + 4);
		m_textures_base.at(0)->setActive();

		preview_fb.bind();
		glViewport(0, 0, 256, 256);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		preview_object.drawMesh(P, V, *shader);
		preview_fb.unbind();

		ImGui::Image((ImTextureID)preview_fb.getTextureID(), ImVec2(100.0, 100.0), ImVec2(0, 1), ImVec2(1, 0));
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	bool load_texture = ImGui::Button("Apply texture");
	if (load_texture)
	{
		string path = m_fd->OpenFile(".jpg");
		if (path != "")
		{
			cout << "Open: " << path << endl;
			addTexture(path);
		}
	}
}






