#pragma once
#include <Engine/Game/GameComponents/CameraComponent.h>
#include <Engine/Game/GameComponents/DirectionalLight.h>
#include <Engine/Game/GameComponents/MeshRendererComponent.h>
#include <Engine/Game/GameComponents/PointLightComponent.h>

#include <Common/StdInclude.h>

namespace BLAengine
{
    class BLACORE_API RenderingManager
    {
    public:
        enum RenderManagerType{ Game = 0, EditorGizmos = 1 };

        RenderingManager(RenderManagerType type);
        ~RenderingManager();

        blaU32 RegisterMeshRenderer(MeshRendererComponent* object);
        bool         CancelMeshRendererTicket(MeshRendererComponent* object);

        blaU32 RegisterDirectionalLight(DirectionalLight* dirLight, CameraComponent* shadowCamera);
        blaU32 CancelDirectionalLightTicket(DirectionalLight* dirLight);

        std::unordered_map<blaU32, MeshRendererComponent*>* GetTicketedMeshRenderers();
        std::unordered_map<blaU32, std::pair<DirectionalLight*, CameraComponent*>>* GetTicketedDirectionalLights();

        void Update();


    private:

        RenderManagerType m_renderManagerType;

        std::unordered_map<blaU32, MeshRendererComponent*> m_ticketedMeshRenderers;
        std::unordered_map<blaU32, pair<DirectionalLight*, CameraComponent*>> m_ticketedDirLights;

        int currentTicket;

    };

    class BLACORE_API DebugRenderingManager
    {
    public:

        DebugRenderingManager() {};
        ~DebugRenderingManager() {};

        vector<pair<vector<blaVec3>, vector<blaVec3>>> m_lineMeshes;

        void LoadDebugLineMesh(pair<vector<blaVec3>, vector<blaVec3>>& lineMesh);

        void Update();
    };




}