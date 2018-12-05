#include "MeshRendererComponent.h"
#include "../GameObject.h"
using namespace BLAengine;

MeshRendererComponent::MeshRendererComponent():
    m_renderType    (GL_TRIANGLES),
    m_renderTicket  (0),
    m_modelTransformMatrix(new blaMat4(0))
{
    this->m_mesh = nullptr;
} 

MeshRendererComponent::~MeshRendererComponent(void)
{
    free(m_modelTransformMatrix);
}

string MeshRendererComponent::ToString()
{
    return "MeshRendererComponent ToString PLACEHOLDER";
}

bool MeshRendererComponent::AssignTriangleMesh(TriangleMesh* mesh)
{
    m_mesh = mesh;

    return true;
}

bool MeshRendererComponent::AssignMaterial(Material* material, int matIndx)
{
    m_materials.push_back(material);

    return true;
}

blaMat4* MeshRendererComponent::GetTransformMatrix() const
{
    blaMat4* a = m_modelTransformMatrix;
    return a;
}

void MeshRendererComponent::Update()
{
    if (!m_parentObject)
        *m_modelTransformMatrix = blaMat4(0);
    else
        m_parentObject->GetTransform().GetScaledTransformMatrix(*m_modelTransformMatrix);
}
