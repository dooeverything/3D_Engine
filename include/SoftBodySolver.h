#pragma once

#include <unordered_map>
#include "Mesh.h"
#include "Particle.h"
#include "make_tet_mesh.h"
#include "make_signed_distance.h"
#include "trimesh.h"
#include "imgui-docking/imgui.h"

//#include "Trimesh.h"

class SoftBodySolver
{
public:
	SoftBodySolver(Mesh* mesh);
	~SoftBodySolver();

	void simulate();
	bool getSimulate() { return m_simulate; };
	
	void renderProperty();

	Mesh* m_mesh;
	unique_ptr<Mesh> m_tet_mesh;

private:
	void getTet(TetMesh& tet_mesh);
	void getTetVertices(const TetMesh& tet_mesh);
	void getEdgeVertices(const TetMesh& tet_mesh);

	void getTetIds();
	void getEdgeIds();

	glm::ivec3 getGridPos(glm::vec3 pos);
	uint getHashIndex(glm::ivec3& pos);

	void buildHash();
	
	void computePredictEdges();
	void computePredictTets();

	void solveDistance(vector<glm::vec3>& predict);
	void solveVolume(vector<glm::vec3>& predict);
	
	vector<uint> m_hash_start;
	vector<uint> m_hash_ids;
	vector<info::VertexLayout> m_layouts;

	vector<shared_ptr<SoftParticle>> m_tets;
	vector<shared_ptr<SoftParticle>> m_edges;

	vector<uint> m_tet_indices;
	vector<uint> m_edges_indices;

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