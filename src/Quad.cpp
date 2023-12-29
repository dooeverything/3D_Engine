#include "Quad.h"
#include "MeshImporter.h"

Quad::Quad()
{
	vector<shared_ptr<Mesh>> mesh;
	shared_ptr<MeshImporter> importer = MeshImporter::create("assets/models/Debug.txt");
	importer->importMesh(mesh);
	m_mesh = mesh.back();
}

Quad* Quad::getQuad()
{
	if (m_quad == nullptr)
	{
		m_quad = unique_ptr<Quad>(new Quad());
	}

	return m_quad.get();
}
