#include "Transform.h"
#include "Quaternion.h"
#include "imgui-docking/imgui.h"

void Transform::setTranslation(const glm::vec3& p)
{
	m_t = p;
	updateTranslation();
}

void Transform::setRotation(const glm::vec3& r)
{
	m_r = r;
	updateRotation();
}

void Transform::setScale(const glm::vec3& s)
{
	m_s = s;
	updateScale();
}

void Transform::updateTranslation()
{
	m_translation = m_translation * glm::translate(glm::mat4(1.0f), m_t);
}

void Transform::updateRotation()
{
	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 axis = glm::vec3(0.0f);
		axis[i] = 1.0f;
		float angle = glm::radians(m_r[i]);

		Quaternion q;
		q.set(axis, angle);
		glm::mat4 m = q.getMatrix();
		m_rotation *= m;
	}
}

void Transform::updateScale()
{
	m_scale = m_scale * glm::scale(glm::mat4(1.0f), glm::abs(m_s));
}

void Transform::renderTranslationPanel()
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Position");

	ImGui::TableNextColumn();
	ImGui::Text("X");
	ImGui::SameLine();
	bool change_x = ImGui::DragFloat("##x1", &(m_t.x), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	bool change_y = ImGui::DragFloat("##y1", &(m_t.y), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	bool change_z = ImGui::DragFloat("##z1", &(m_t.z), 0.005f);

	if (change_x || change_y || change_z)
	{
		updateTranslation();
	}

	ImGui::TableNextColumn();
}

void Transform::renderRotationPanel()
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Rotation");

	ImGui::TableNextColumn();
	ImGui::Text("X");
	ImGui::SameLine();
	bool change_x = ImGui::DragFloat("##x2", &(m_r.x), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	bool change_y = ImGui::DragFloat("##y2", &(m_r.y), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	bool change_z = ImGui::DragFloat("##z2", &(m_r.z), 0.005f);

	if (change_x || change_y || change_z)
	{
		updateRotation();
	}

	ImGui::TableNextColumn();
}

void Transform::renderScalePanel()
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Scale");
	ImGui::TableNextColumn();

	ImGui::Text("X");
	ImGui::SameLine();
	bool change_x = ImGui::DragFloat("##x3", &(m_s.x), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Y");
	ImGui::SameLine();
	bool change_y = ImGui::DragFloat("##y3", &(m_s.y), 0.005f);

	ImGui::TableNextColumn();
	ImGui::Text("Z");
	ImGui::SameLine();
	bool change_z = ImGui::DragFloat("##z3", &(m_s.z), 0.005f);

	if (change_x || change_y || change_z)
	{
		updateScale();
	}

	ImGui::TableNextColumn();
}