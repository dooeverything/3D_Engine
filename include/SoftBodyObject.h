#pragma once

#ifndef SOFTBODYOBJECT_H
#define SOFTBODYOBJECT_H

#include <unordered_map>

#include "glm/gtx/string_cast.hpp"
#include "imgui-docking/imgui.h"
#include "make_tet_mesh.h"
#include "make_signed_distance.h"
#include "trimesh.h"

#include "Mesh.h"
#include "Object.h"

class SoftParticle;
class Light;
class Transform;

class SoftBodyObject : public Object
{
public:
	SoftBodyObject(
		const vector<info::VertexLayout>& vertices,
		const vector<info::uint>& indices,
		const Transform& transform,
		const glm::vec3& b_min, const glm::vec3& b_max
		);
	~SoftBodyObject();

	virtual void draw(
		const glm::mat4& P,
		const glm::mat4& V,
		const glm::vec3& view_pos,
		const Light& light) override;

	void simulate();

	inline void setSimulate(bool simulate) { m_simulate = simulate; };

	inline bool getSimulate() { return m_simulate; };

private:
	vector<glm::vec3> transformVertices(
		const vector<info::VertexLayout>& vertices);

	void getTet(
		const vector<glm::vec3>& vertices,
		const vector<info::uint>& indices,
		const glm::vec3& b_min, 
		const glm::vec3& b_max);
	
	void getTetVertices(const TetMesh& tet_mesh);
	void solveDistance(vector<glm::vec3>& predict);
	void solveVolume(vector<glm::vec3>& predict);

	void reset();
	
	vector<shared_ptr<SoftParticle>> m_tets;
	vector<info::VertexLayout> m_tet_vertices_og;
	vector<info::VertexLayout> m_tet_vertices;
	vector<info::uint> m_tet_indices;
	vector<info::uint> m_faces;
	vector<float> m_rest_d;
	vector<float> m_rest_v;
	
	float m_dx;
	float t;
	float n_sub_steps;
	float t_sub;
	
	bool m_simulate;
	bool m_reset;
};

#endif // !SOFTBODYOBJECT_H