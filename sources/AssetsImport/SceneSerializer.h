#pragma once

/*Serialization Includes: */
#include <cereal\cereal.hpp>
#include <cereal\types\polymorphic.hpp>
#include <cereal\types\vector.hpp>
#include <cereal\types\string.hpp>
#include <cereal\archives\json.hpp>

#include "../Engine/Game/Scene.h"
#include "MathSerializer.h"

class TransformSerializer
{
public:

    TransformSerializer() = default;

    void FromTransform(BLAengine::ObjectTransform* transform)
    {
        m_position.FillData(transform->GetPosition());
        m_rotation.FillData(transform->GetRotation());
        m_scale.FillData(transform->m_scale);
    }
    void ToTransform(BLAengine::ObjectTransform* transform)
    {
        blaVec3 position;
        m_position.LoadData(position);
        transform->SetPosition(position);

        blaQuat rotation;
        m_rotation.LoadData(rotation);
        transform->SetRotation(rotation);

        m_scale.LoadData(transform->m_scale);
    }

private:
    friend class cereal::access;

    vec3serializer m_position;
    quatserializer m_rotation;
    vec3serializer m_scale;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            cereal::make_nvp("position", m_position),
            cereal::make_nvp("rotation", m_rotation),
            cereal::make_nvp("scale", m_scale)
        );
    }
};

class GameComponentSerializer
{
public:

    GameComponentSerializer() = default;

    virtual void FromGameComponent(BLAengine::GameComponent* comp)
    {}
    virtual void ToGameComponent(BLAengine::GameComponent* comp)
    {}

private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive & archive)
    {}
};

class MeshRendererSerializer : public GameComponentSerializer
{
public:

    MeshRendererSerializer() = default;

    void FromGameComponent(BLAengine::GameComponent* comp)
    {
        GameComponentSerializer::FromGameComponent(comp);
        if (BLAengine::MeshRendererComponent* meshRender = dynamic_cast<BLAengine::MeshRendererComponent*>(comp))
        {
            m_triangleMeshName = meshRender->m_mesh->GetName();

            for (auto material : meshRender->m_materials)
            {
                m_materialNames.push_back(material->GetName());
            }
        }
    }
    
    std::string GetMeshName() { return m_triangleMeshName; }
    
    vector<std::string> GetMaterialNames() { return m_materialNames; }

private:
    friend class cereal::access;

    string m_triangleMeshName;
    vector<string> m_materialNames;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            cereal::make_nvp("TriangleMesh", m_triangleMeshName),
            cereal::make_nvp("Material", m_materialNames)
        );
    }
};

class DirectionalLightSerializer : public GameComponentSerializer
{
public:

    DirectionalLightSerializer() = default;

    void FromGameComponent(BLAengine::GameComponent* comp)
    {
        if(BLAengine::DirectionalLight* dirLight = dynamic_cast<BLAengine::DirectionalLight*>(comp))
            m_lightDirection.FillData(dirLight->GetDirection());
    }
    void ToGameComponent(BLAengine::GameComponent* comp)
    {
        blaVec3 dir;
        m_lightDirection.LoadData(dir);

        if (BLAengine::DirectionalLight* dirLight = dynamic_cast<BLAengine::DirectionalLight*>(comp))
            dirLight->SetDirection(dir);
    }

private:
    friend class cereal::access;

    vec3serializer m_lightDirection;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            //cereal::make_nvp("gameObject", cereal::base_class<GameObjectSerializer>(this)),
            cereal::make_nvp("lightDirection", m_lightDirection)
        );
    }
};

class RigidBodySerializer : public GameComponentSerializer
{
public:

    RigidBodySerializer() = default;

    void FromGameComponent(BLAengine::GameComponent* comp)
    {
        if (BLAengine::RigidBodyComponent* rgb = dynamic_cast<BLAengine::RigidBodyComponent*>(comp))
            m_mass.FillData(rgb->m_massTensor);
    }
    void ToGameComponent(BLAengine::GameComponent* comp)
    {
        if (BLAengine::RigidBodyComponent* rgb = dynamic_cast<BLAengine::RigidBodyComponent*>(comp))
            m_mass.LoadData(rgb->m_massTensor);
    }

private:
    friend class cereal::access;

    mat3serializer m_mass;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            cereal::make_nvp("MassTensor", m_mass)
        );
    }
};

class GameObjectSerializer
{
public:

    GameObjectSerializer() = default;

    void FromGameObject(BLAengine::GameObject* gobject)
    {
        BLAengine::ObjectTransform transform = gobject->GetTransform();
        m_transform.FromTransform(&transform);

        m_objectName = gobject->GetName();

        vector<BLAengine::MeshRendererComponent*> meshRenderers = gobject->GetComponents<BLAengine::MeshRendererComponent>();
        for (size_t i = 0; i < meshRenderers.size(); i++)
        {
            BLAengine::MeshRendererComponent* mrenderer = meshRenderers[i];
            MeshRendererSerializer serializer;
            serializer.FromGameComponent(mrenderer);

            std::shared_ptr<GameComponentSerializer> gCompSerializer = std::make_shared<MeshRendererSerializer>(serializer);
            m_componentsVector.push_back(gCompSerializer);
        }

        vector<BLAengine::DirectionalLight*> dirLights = gobject->GetComponents<BLAengine::DirectionalLight>();
        for (size_t i = 0; i < dirLights.size(); i++)
        {
            BLAengine::DirectionalLight* mrenderer = dirLights[i];
            DirectionalLightSerializer serializer;
            serializer.FromGameComponent(mrenderer);

            std::shared_ptr<GameComponentSerializer> gCompSerializer = std::make_shared<DirectionalLightSerializer>(serializer);
            m_componentsVector.push_back(gCompSerializer);
        }

        vector<BLAengine::RigidBodyComponent*> rgbs = gobject->GetComponents<BLAengine::RigidBodyComponent>();
        for (size_t i = 0; i < rgbs.size(); i++)
        {
            BLAengine::RigidBodyComponent* mrenderer = rgbs[i];
            RigidBodySerializer serializer;
            serializer.FromGameComponent(mrenderer);

            std::shared_ptr<GameComponentSerializer> gCompSerializer = std::make_shared<RigidBodySerializer>(serializer);
            m_componentsVector.push_back(gCompSerializer);
        }
    }
    
    BLAengine::ObjectTransform GetTransform()
    {
        BLAengine::ObjectTransform t;
        m_transform.ToTransform(&t);
        return t;
    }

    std::vector<std::shared_ptr<GameComponentSerializer>> GetComponentVector() { return m_componentsVector; }

    std::string GetName() { return m_objectName; }

private:

    string m_objectName;
    TransformSerializer m_transform;

    std::vector<std::shared_ptr<GameComponentSerializer>> m_componentsVector;

    friend class cereal::access;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            cereal::make_nvp("Name", m_objectName),
            cereal::make_nvp("ObjectTransform", m_transform),
            cereal::make_nvp("Components", m_componentsVector)
        );
    }

};

class SceneSerializer
{
public:

    SceneSerializer() = default;

    void FromScene(BLAengine::Scene* scene)
    {
        std::vector<BLAengine::GameObject*> objVec = scene->GetObjects();
        
        for (size_t i = 0; i < objVec.size(); i++)
        {
            BLAengine::GameObject* obj = objVec[i];

            GameObjectSerializer gObjSerializer;

            gObjSerializer.FromGameObject(obj);

            m_objectsVector.push_back(gObjSerializer);
        }
    }

    std::vector<GameObjectSerializer> GetGameObjectVector() { return m_objectsVector; }
    std::string GetName() { return m_sceneName; }
private:
    friend class cereal::access;

    std::vector<GameObjectSerializer> m_objectsVector;
    std::string m_sceneName;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive
        (
            cereal::make_nvp("Objects", m_objectsVector)
        );
    }
};

CEREAL_REGISTER_TYPE(MeshRendererSerializer)
CEREAL_REGISTER_TYPE(DirectionalLightSerializer)
CEREAL_REGISTER_TYPE(RigidBodySerializer)

CEREAL_REGISTER_POLYMORPHIC_RELATION(GameComponentSerializer, MeshRendererSerializer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(GameComponentSerializer, DirectionalLightSerializer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(GameComponentSerializer, RigidBodySerializer)