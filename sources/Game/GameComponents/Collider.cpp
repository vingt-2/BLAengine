#include "Collider.h"

Collider::Collider(Transform* parentTransform):
	m_meshVertices(vector<vec3>()),
	m_colliderRenderer(new MeshRenderer(parentTransform))
{
	m_parentTransform = parentTransform;
	m_colliderRenderer->m_renderType = 0x0003;
}

Collider::~Collider()
{
}

void Collider::GenerateBoundingBoxFromMesh(MeshRenderer* meshRenderer)
{
	cout << "\n [Collider] Generating Bounding Box From Mesh: ";

	GLfloat maxX, maxY, maxZ, minX, minY, minZ;

	for(GLuint i = 0; i<meshRenderer->m_meshVertices.size(); i++)
	{
		vec3 vertex = meshRenderer->m_meshVertices.at(i);
		if(i == 0)
		{
			maxX = vertex.x;
			minX = vertex.x;
			maxY = vertex.y;
			minY = vertex.y;
			maxZ = vertex.z;
			minZ = vertex.z;
		}
		else
		{
			if( vertex.x > maxX )
			{
				maxX = vertex.x;
			}
			if( vertex.x < minX )
			{
				minX = vertex.x;
			}
			if( vertex.y > maxY )
			{
				maxY = vertex.y;
			}
			if( vertex.y < minY )
			{
				minY = vertex.y;
			}
			if( vertex.z > maxZ )
			{
				maxZ = vertex.z;
			}
			if( vertex.z < minZ )
			{
				minZ = vertex.z;
			}
		}
	}

	cout << "(" << minX << "," << minY << "," << minZ << ")" <<  "(" << maxX << "," << maxY << "," << maxZ << ")";
	m_meshVertices.push_back(vec3(maxX,maxY,maxZ));
	m_meshVertices.push_back(vec3(maxX,maxY,minZ));
	m_meshVertices.push_back(vec3(maxX,minY,maxZ));
	m_meshVertices.push_back(vec3(maxX,minY,minZ));
	m_meshVertices.push_back(vec3(minX,maxY,maxZ));
	m_meshVertices.push_back(vec3(minX,maxY,minZ));
	m_meshVertices.push_back(vec3(minX,minY,maxZ));
	m_meshVertices.push_back(vec3(minX,minY,minZ));
}

void Collider::UpdateRenderer()
{
	m_colliderRenderer->CleanUp();
	m_colliderRenderer->GenerateVertexArrayID();
	m_colliderRenderer->m_meshVertices = m_meshVertices;
	m_colliderRenderer->AssignMaterial("defaultShader");
	m_colliderRenderer->GenerateArrays();
}