#pragma once

class ImGuiPanel
{
public:
	ImGuiPanel();
	~ImGuiPanel();
	virtual void render();
};

class PropertyPanel : public ImGuiPanel
{
public:
	PropertyPanel();
	~PropertyPanel();

private:


};