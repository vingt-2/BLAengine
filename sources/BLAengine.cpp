#include "Common/StdInclude.h"
#include "Engine/System/RenderWindow.h"
#include "AnimationDemo.h"
#include "./Common/DataStructures/Tree.h"

#ifdef BLA_NO_DLL
using namespace BLAengine;

BLA_DECLARE_SINGLETON(EngineInstance)

void DoAnimationDemoSession()
{
    BLA_ASSIGN_SINGLETON(EngineInstance, new AnimationDemo(true, false));

    EngineInstance* demo;

    BLA_RETRIEVE_SINGLETON(EngineInstance, demo);

    auto renderWindow = new GLFWRenderWindow();

    renderWindow->CreateRenderWindow("BLAengine Animation Demo", 1280, 720, false);

    demo->InitializeEngine(renderWindow);

    demo->LoadWorkingScene("./Scenes/empty_scene");

    while (!demo->ShouldTerminate())
    {
        demo->PreEngineUpdate();
        demo->EngineUpdate();
        demo->PostEngineUpdate();
    }

    // Call terminates engine
    demo->TerminateEngine();
}

int main()
{
    DoAnimationDemoSession();
}

#endif