// BLAEngine Copyright (C) 2016-2020 Vincent Petrella. All rights reserved
#pragma once

#include "Core/InspectableVariables.h"
#include "Gpu/Image.h"
#include "Gpu/Resource.h"
#include "Rendering/RenderPass.h"
#include "System/RenderWindow.h"
#include "Pointer.h"

namespace BLA
{
    using BLARgba = blaU32;
    
    struct OffscreenRenderTarget
    {
        blaOwnedPtr<Gpu::Image<Gpu::Formats::R8G8B8A8_UNORM>> m_color;
    };

    class RenderWindow;
    class Renderer
    {
    public:
        Renderer(GLFWRenderWindow* pRenderWindow);

        BLACORE_API bool Update();

        //TODO: Remove, improve DevGuiwindow design ...
        void SetViewportSize(blaIVec2 viewportSize);
        RenderWindow* GetRenderWindow();
        ~Renderer(void) = default;

        OffscreenRenderTarget m_offscreenBuffer;

        void SetupRenderPassInstances();

    protected:
        blaU32 SetupAllRegisteredRenderPasses();

        void CreateOrUpdateRenderTargets();
        GLFWRenderWindow* m_renderWindow;
        blaIVec2 m_viewPortExtents;

        bool m_firstUpdate = true;
    };
}
