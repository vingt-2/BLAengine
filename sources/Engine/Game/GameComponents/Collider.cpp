#include "Collider.h"

Collider::Collider(MeshRenderer* mesh)
{
	m_triVertices = &(mesh->m_meshVertices);
	m_triNormals = &(mesh->m_meshNormals);

	m_collisionMesh = newCollisionModel3D();
	for (int i = 0; i < mesh->m_meshTriangles.size(); i += 3)
	{
		uint32_t index = mesh->m_meshTriangles[i];

		if (index + 2 < mesh->m_meshVertices.size())
		{
			m_collisionMesh->addTriangle(&(mesh->m_meshVertices[index + 0].x),
				&(mesh->m_meshVertices[index + 1].x),
				&(mesh->m_meshVertices[index + 2].x));
		}
	}
	m_collisionMesh->finalize();
}

Collider::~Collider()
{
	m_collisionMesh->~CollisionModel3D();
}