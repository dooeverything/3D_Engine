#pragma once

#ifndef SOFTBODYSOLVER_H
#define SOFTBODYSOLVER_H

#include <unordered_map>

#include "imgui-docking/imgui.h"
#include "make_tet_mesh.h"
#include "make_signed_distance.h"
#include "trimesh.h"

#include "Mesh.h"
//#include "Particle.h"

class SoftParticle;

class SoftBodySolver
{
public:
	SoftBodySolver(Mesh* mesh);
	~SoftBodySolver();

	void simulate();
	void renderProperty();

	bool getSimulate() { return m_simulate; };

private:
	glm::ivec3 getGridPos(glm::vec3 pos);
	info::uint getHashIndex(glm::ivec3& pos);

	void getTet(TetMesh& tet_mesh);
	void getTetVertices(const TetMesh& tet_mesh);
	void getEdgeVertices(const TetMesh& tet_mesh);
	void getTetIds();
	void getEdgeIds();
	void buildHash();
	void computePredictEdges();
	void computePredictTets();
	void solveDistance(vector<glm::vec3>& predict);
	void solveVolume(vector<glm::vec3>& predict);
	
	void reset();

	Mesh* m_mesh;
	unique_ptr<Mesh> m_tet_mesh;
	vector<info::uint> m_hash_start;
	vector<info::uint> m_hash_ids;
	
	vector<info::VertexLayout> m_layouts;
	//const vector<info::VertexLayout>& m_og_layouts;

	vector<shared_ptr<SoftParticle>> m_tets;
	vector<shared_ptr<SoftParticle>> m_edges;

	vector<info::uint> m_tet_indices;
	vector<info::uint> m_edges_indices;

	vector<float> m_rest_d;
	vector<float> m_rest_v;
	
	glm::ivec3 m_size;
	glm::ivec3 m_max;
	glm::ivec3 m_min;
	glm::vec3 m_spacing;
	
	int m_hash_size;

	float m_dx;
	float t;
	float n_sub_steps;
	float t_sub;
	
	bool m_simulate;
};

#endif // !SOFTBODYSOLVER_H