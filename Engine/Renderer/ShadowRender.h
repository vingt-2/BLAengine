// BLAEngine Copyright (C) 2016-2020 Vincent Petrella. All rights reserved.

#pragma once
#include "System.h"
#include "RenderBackend.h"

#include "RenderCamera.h"

namespace BLA
{

    struct ShadowRender
    {
        virtual blaMat4 getShadowViewProjection() = 0;
        virtual void Update() = 0;  // <-- m_shadowCamera.Update();
    };

    struct DirectionalShadowRender : ShadowRender
    {
    public:
        blaVec3 m_shadowDirection;

        //OpenGL
        GLuint m_depthTexture;
        GLuint m_shadowPrgmID;
        GLuint m_shadowBuffer;

        int m_bufferSize;

        blaMat4 getShadowViewProjection();
        void Update();

        OrthographicCamera m_shadowCamera;
    };

    struct PerspectiveShadowRender : ShadowRender
    {
    public:
        blaVec3 m_shadowDirection;

        //OpenGL
        GLuint m_depthTexture;
        GLuint m_shadowPrgmID;
        GLuint m_shadowBuffer;

        int m_bufferSize;

        blaMat4 getShadowViewProjection();
        void Update();

        OrthographicCamera m_shadowCamera;

    };

    struct Cubemap : ShadowRender
    {
    public:
        blaVec3 m_shadowDirection;

        //OpenGL
        GLuint m_depthTexture[6];
        GLuint m_shadowPrgmID;
        GLuint m_shadowBuffers[6];

        int m_bufferSize;

        blaMat4 getShadowViewProjection();
        void Update();

        PerspectiveCamera m_shadowCamera;
    };


}