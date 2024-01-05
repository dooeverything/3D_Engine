#include "ImGuiButton.h"

#include "Texture.h"

ImGuiButton::ImGuiButton(const string& path, const string& name) :
	m_path(path), m_name(name), m_press(false)
{
	m_texture = make_unique<Texture>(path);
	m_texture->loadTexture();
}

ImGuiButton::~ImGuiButton() {}

bool ImGuiButton::draw()
{
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
	ImVec4 bg_col = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);             // Black background
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
	
	bool button_click = false;
	if (m_press)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.8f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.8f, 0.8f, 1.0f });
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 74.0f / 255.0f,76.0f / 255.0f,71.0f / 255.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
	}

	if (ImGui::ImageButton(m_name.c_str(), (ImTextureID)m_texture->getTextureID(), ImVec2(16, 16),
		uv_min, uv_max, bg_col, tint_col))
	{
		m_press = !m_press;
		button_click = true;
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	return button_click;
}

ImGuiButtonTransform::ImGuiButtonTransform()
{
	m_button_cursor = make_unique<ImGuiButton>("assets/textures/ArrowNone.png", "cursor");
	m_button_cursor->setPress(true);
	m_button_translation = make_unique<ImGuiButton>("assets/textures/ArrowTranslation.png", "translation");
	m_button_scale = make_unique<ImGuiButton>("assets/textures/ArrowScale.png", "scale");
}

ImGuiButtonTransform::~ImGuiButtonTransform() {}

void ImGuiButtonTransform::draw(ImVec2 pos)
{
	ImGui::SetCursorPos(pos);

	if (m_button_cursor->draw())
	{
		if (m_button_cursor->getPress())
		{
			m_button_translation->setPress(false);
			m_button_scale->setPress(false);
		}
		else
		{
			m_button_cursor->setPress(true);
		}
	}

	ImGui::SameLine();

	if (m_button_translation->draw())
	{
		if (m_button_translation->getPress())
		{
			m_button_cursor->setPress(false);
			m_button_scale->setPress(false);
		}
		else
		{
			m_button_cursor->setPress(true);
		}
	}
	
	ImGui::SameLine();

	if (m_button_scale->draw())
	{
		if (m_button_scale->getPress())
		{
			m_button_cursor->setPress(false);
			m_button_translation->setPress(false);
		}
		else
		{
			m_button_cursor->setPress(true);
		}
	}

}

int ImGuiButtonTransform::getTransformationType()
{
	if (m_button_cursor->getPress())
		return -1;

	if (m_button_translation->getPress())
		return 0;

	if (m_button_scale->getPress())
		return 1;

	return -1;
}

ImGuiButtonPlay::ImGuiButtonPlay()
{
	m_button_play  = make_unique<ImGuiButton>("assets/textures/Play.png", "play");
	m_button_pause = make_unique<ImGuiButton>("assets/textures/Pause.png", "pause");
}

ImGuiButtonPlay::~ImGuiButtonPlay() {}

void ImGuiButtonPlay::draw(ImVec2 pos)
{
	ImGui::SetCursorPos(pos);

	if (m_button_play->draw())
	{
		// if button click
		if (m_button_play->getPress())
		{
			m_button_pause->setPress(false);
		}
		else
		{
			m_button_play->setPress(false);
			m_button_pause->setPress(false);
		}
	}
	
	ImGui::SameLine();

	if (m_button_pause->draw())
	{
		if (m_button_pause->getPress())
		{
			m_button_play->setPress(false);
		}
		else
		{
			m_button_play->setPress(true);
		}
	}
}
