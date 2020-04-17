#pragma once
#include <Maths/Maths.h>
#include "Core/GameComponent.h"

namespace BLAengine
{
    BeginComponentDeclaration(BLAEngineNative, TransformComponent)

        blaScaledTransform GetTransform();
        blaScaledTransform GetLocalTransform();

        void SetTransform(const blaScaledTransform& transform);
        void SetLocalTransform(const blaScaledTransform& transform);

    private:
        blaScaledTransform m_localTransform;
        blaS32 m_setCounter = 0;

        blaScaledTransform m_cachedWorldTransform;
        blaS32 m_cachedParentSetCounter = -1;
        blaBool m_dirtyCachedTransform = true;

    EndComponentDeclaration();
}