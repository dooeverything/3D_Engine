#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Material.h"
#include "Object.h"

class Light;

class GameObject : public Object
{
public:
	GameObject();
	GameObject(const string& mesh_path);
	GameObject(const string& mesh_path, const vector<string>& shader_path);
	~GameObject();

	virtual void move(Camera& camera);
	virtual void drawPreview(const Material& mat);
	virtual void drawPreview(const vector<shared_ptr<Texture>>& tex);
	virtual void draw(const glm::mat4& P, const glm::mat4& V,
					Light& light, glm::vec3& view_pos, ShadowMap& shadow,
					IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut);
	virtual void drawInstance(glm::mat4& P, glm::mat4& V);

	// Getter
	virtual inline int getMoveAxis() { return m_move_axis; };
	virtual inline bool getIsPopup() { return m_is_popup; };

	// Setter
	virtual inline void setMoveAxis(int axis) { m_move_axis = axis; };
	virtual inline void setIsPopup(bool p) { m_is_popup = p; };
	virtual inline void setColor(glm::vec3 color) { m_color = color; };

	virtual void renderProperty() { return; };
	virtual void updateVertex(glm::vec3 ray_dir, glm::vec3 ray_pos, bool mouse_down) { return; };

private:
	glm::vec3 m_color;
	bool m_is_popup;
	int m_move_axis;
};

#endif // !GAMEOBJECT_H


