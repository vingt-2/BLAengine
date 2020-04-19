#pragma once
#include <Maths/Maths.h>
#include <Core/GameComponent.h>

namespace BLAengine
{
    BeginComponentDeclaration(BLAEngineNative, DirectionalLightComponent)
        void SetDirection(blaVec3 direction);
        blaVec3 GetDirection();

        void Init() override;
        void Shutdown() override;
        void Update();

        blaU32 m_renderTicket = 0;

    private:
        blaVec3 m_lightDirection = blaVec3(0.f);

    EndComponentDeclaration()
}
