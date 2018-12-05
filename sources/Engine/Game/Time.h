#pragma once
#include "..\..\Common\StdInclude.h"
#include "..\..\Common\System.h"
#include "..\..\Common\Maths\Maths.h"

namespace BLAengine
{
    class BLACORE_API Time
    {
    public:
        Time(glm::uint8 timeBufferSize);
        ~Time();

        float GetTime();
        float GetDelta();
        float GetFramerate();

        void Update();

    private:

        float m_time;
        float m_elapsedTime;

        vector<float> m_timeBuffer;
        glm::uint8 m_timeBufferSize;
    };
}
