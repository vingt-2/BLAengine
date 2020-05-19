#include "SceneEditor.h"

#include "System.h"
#include "Maths/Maths.h"
#include "Renderer/GL33Renderer.h"
#include "Core/Timer.h"
#include "Core/DebugDraw.h"
#include "Assets/SceneManager.h"
#include "CameraControl.h"
#include "System/Console.h"
#include "System/InputManager.h"
#include "Gui/GuiManager.h"
#include "Gui/GuiMenu.h"
#include "Physics/ColliderComponent.h"
#include "Geometry/PrimitiveGeometry.h"
#include "System/FileSystem/Files.h"
#include "Renderer/MeshRendererComponent.h"
#include "EditorComponentLibrariesManager.h"
#include "AssetsImport/OBJImport.h"
#include "Core/TransformComponent.h"
#include "Assets/Texture.h"

#include "System/RenderWindow.h"
#include "Renderer/VulkanRenderer.h"

using namespace BLA;

DeclareConsoleVariable(bool, shouldReloadLibraries, false)

DefineConsoleCommand(void, MakeSkyObject)
{
    Asset* m;
    if (AssetManager::GetSingletonInstance()->GetAsset("SkySphere", m) != AssetManager::TriangleMeshAsset)
    {
        MeshAsset skyInvertedSphere = MeshAsset("SkySphere");
        skyInvertedSphere.m_triangleMesh = PrimitiveGeometry::MakeSphere(5000, true);
        skyInvertedSphere.m_triangleMesh.m_materials.push_back(std::make_pair("BlankDiffuseMat", 0));
        AssetManager::GetSingletonInstance()->SaveTriangleMesh(&skyInvertedSphere);
        AssetManager::GetSingletonInstance()->LoadTriangleMesh("SkySphere");
    }
    GameObject skySphereObject = Scene::GetSingletonInstance()->CreateObject(BlaStringId("Sky Sphere"));

    skySphereObject.CreateComponent<MeshRendererComponent>()->m_meshAssetName = "SkySphere";
    //skySphereObject.GetComponent<MeshRendererComponent>()->MaterialName = "BlankDiffuseMat";
}

void DragAndDropHandler(DragAndDropPayloadDontStore* dragAndDropInput)
{
    for (auto path : *dragAndDropInput)
    {
        Console::LogMessage("Dropped file " + path);
        static_cast<SceneEditor*>(EngineInstance::GetSingletonInstance())->EditorDragAndDropedFile(path);
    }
}

namespace BLA
{
    struct EditorState
    {
        enum StateType
        {
            BLA_EDITOR_EDITING,
            BLA_EDITOR_LOAD_SCENE_PROMPT,
            BLA_EDITOR_SAVE_SCENE_PROMPT
        } m_type = BLA_EDITOR_EDITING;
    };

    struct EditorLoadSceneState : EditorState
    {
        EditorLoadSceneState()
        {
            m_type = BLA_EDITOR_LOAD_SCENE_PROMPT;
            m_currentFileBrowser = nullptr;
        }

        const OpenFilePrompt* m_currentFileBrowser;
    };

    struct EditorSaveSceneState : EditorState
    {
        EditorSaveSceneState()
        {
            m_type = BLA_EDITOR_SAVE_SCENE_PROMPT;
            m_currentFileBrowser = nullptr;
        }

        const SaveFilePrompt* m_currentFileBrowser;
    };
}


void SceneEditor::SetEditorState(EditorState* state)
{
    delete m_editorState;
    m_editorState = state;
}

void SceneEditor::PreEngineUpdate()
{
    if(g_shouldReloadLibraries) 
    {
        static_cast<EditorComponentLibrariesManager*>(m_componentLibrariesManager)->ReloadLibraries();
        g_shouldReloadLibraries = false;
    }

	if(m_bTestVulkan)
	{
		if(!m_vulkanWindow && !m_vulkanRenderer)
		{
            m_vulkanWindow = new GLFWRenderWindow();
            m_vulkanWindow->CreateVulkanRenderWindow("Vulkan Window", 100, 100, false);
            m_vulkanRenderer = new VulkanRenderer();
            m_vulkanRenderer->InitializeRenderer(m_vulkanWindow, m_renderingManager, m_debugRenderingManager);
		}
	}
	if(m_vulkanRenderer && m_vulkanWindow)
	{
		if(!m_bTestVulkan)
		{
            delete m_vulkanWindow;
            m_vulkanWindow = nullptr;
            delete m_vulkanRenderer;
            m_vulkanRenderer = nullptr;
		}
	}

    EngineInstance::PreEngineUpdate();

    if (m_editorState->m_type == EditorState::BLA_EDITOR_EDITING)
    {
        if (m_cameraController)
        {
            m_cameraController->UpdateController(m_timer->GetDelta());
        }
    }
    else if (m_editorState->m_type == EditorState::BLA_EDITOR_LOAD_SCENE_PROMPT)
    {
        HandleLoadScenePrompt();
    }
    else if (m_editorState->m_type == EditorState::BLA_EDITOR_SAVE_SCENE_PROMPT)
    {
        HandleSaveScenePrompt();
    }

    if (InputManager::GetSingletonInstance()->GetKeyState(BLA_KEY_GRAVE_ACCENT).IsRisingEdge())
    {
        m_editorGuiRequests.m_openConsoleRequest = !m_editorGuiRequests.m_openConsoleRequest;
    }
}

void SceneEditor::EngineUpdate()
{
    if (m_editorState->m_type == EditorState::BLA_EDITOR_EDITING)
    {
        EditorUpdate();

        if (m_sceneGraphGui && (m_scene->GetSceneFlags() & Scene::ESceneFlags::DIRTY_SCENE_STRUCTURE || m_updatedScene) != 0)
        {
            m_sceneGraphGui->UpdateSceneGraph();
            m_updatedScene = false;
        }

        EngineInstance::EngineUpdate();

        HandleEditorStateChangeRequests();

        HandleGuiRequests();

        if (m_inputManager->GetKeyState(BLA_KEY_ESCAPE).IsRisingEdge())
        {
            m_selectedObject = GameObject::InvalidReference();
        }
    }
}

//TODO: Don't call the same things as Engine ...
void SceneEditor::PostEngineUpdate()
{
    m_gizmoManager.Update();

    // TODO: Update() should not be exported, and call from with the engine dll
    m_debug->Update();

    // TODO: Update() should not be exported, and call from with the engine dll
    m_renderer->Update();

	/*
	 * Vulkan Test
	 */
    if (m_vulkanRenderer) m_vulkanRenderer->Update();
	
	// TODO: Update() should not be exported, and call from with the engine dll
    m_guiManager->Update();

    // TODO: Update() should not be exported, and call from with the engine dll
    // Inputs should be the second to last thing to update !
    m_inputManager->Update();

    if (!m_isCapturedMouse)
    {
        m_inputManager->SetKeyboardLock(m_guiManager->IsMouseOverGui());
        m_inputManager->SetMouseLock(m_guiManager->IsMouseOverGui());

        if (const BlaGuiRenderWindow * renderGuiWindow = dynamic_cast<const BlaGuiRenderWindow*>(m_guiManager->OpenWindow("Editor Window")))
        {
            if (renderGuiWindow->HasFocus())
            {
                blaVec2 mouseCoord = renderGuiWindow->GetMousePointerScreenSpaceCoordinates();
                if (mouseCoord.x >= 0.f && mouseCoord.x <= 1.f && mouseCoord.y >= 0.f && mouseCoord.y <= 1.f)
                {
                    m_inputManager->SetKeyboardLock(false);
                    m_inputManager->SetMouseLock(false);
                }

                if(m_inputManager->GetMouseButtonState(BLA_MOUSE_BUTTON_RIGHT).IsDown() ||
                    m_inputManager->GetMouseButtonState(BLA_MOUSE_BUTTON_MIDDLE).IsDown())
                {
                    ToggleCaptureMouse();
                }
            }
        }
    }
    else
    {
        if (m_inputManager->GetMouseButtonState(BLA_MOUSE_BUTTON_RIGHT).IsUp() &&
            m_inputManager->GetMouseButtonState(BLA_MOUSE_BUTTON_MIDDLE).IsUp())
        {
            ToggleCaptureMouse();
        }
    }

    // Final update of the frame
    m_renderWindow->UpdateWindowAndBuffers();

	/*
	 * Vulkan Test
	 */
    if (m_vulkanWindow) m_vulkanWindow->UpdateWindowAndBuffers();
}

bool SceneEditor::InitializeEngine(RenderWindow* renderWindow)
{
    if (EngineInstance::InitializeEngine(renderWindow))
    {
        m_renderer->SetRenderToFrameBufferOnly(true);

        m_guiManager->OpenWindow("Editor Window", new BlaGuiRenderWindow(m_renderer, "Editor Window", blaVec2(0.f, 0.f)));

        m_renderWindow->SetDragAndDropCallback((DragAndDropCallback)DragAndDropHandler);

        m_editorState = new EditorState();
        m_editorState->m_type = EditorState::BLA_EDITOR_EDITING;

        /*
         * Create The menu
         */
        BlaGuiMenuTab& fileMenu = m_guiManager->m_menuBar.AddSubMenu("File");
        fileMenu.AddMenuItem("New Scene", &m_editorStateRequests.m_newSceneRequest);
        fileMenu.AddMenuItem("Open Scene", &m_editorStateRequests.m_openSceneRequest, true);
        fileMenu.AddMenuItem("Save", &m_editorStateRequests.m_saveSceneRequest);
        fileMenu.AddMenuItem("Save As", &m_editorStateRequests.m_saveSceneAsRequest, true);
        fileMenu.AddMenuItem("Exit", &m_isTerminationRequested);


        BlaGuiMenuTab& settingsMenu = m_guiManager->m_menuBar.AddSubMenu("Settings");
        settingsMenu.AddMenuItem("Render G-Buffer", &m_renderer->m_debugDrawGBuffer);
        settingsMenu.AddMenuItem("Draw Grid", &m_bDrawGrid);
        settingsMenu.AddMenuItem("Test Vulkan", &m_bTestVulkan);

        BlaGuiMenuTab& windowsMenu = m_guiManager->m_menuBar.AddSubMenu("Windows");
        windowsMenu.AddMenuItem("Console", &m_editorGuiRequests.m_openConsoleRequest);
        windowsMenu.AddMenuItem("Scene Graph", &m_editorGuiRequests.m_openScenGraphGuiRequest);
        windowsMenu.AddMenuItem("Component Inspector", &m_editorGuiRequests.m_openComponentInspectorRequest);

        LoadNewScene();

        return true;
    }
    return false;
}

bool SceneEditor::LoadNewScene()
{
    EngineInstance::LoadNewScene();

    m_selectedObject = GameObject::InvalidReference();

    // MakeSkyObject();

    delete m_cameraController;
    m_cameraController = new CameraController(
        m_renderWindow,
        100.f,
        10.0f);

    SetEditorState(new EditorState());

    return true;
}

bool SceneEditor::LoadWorkingScene(blaString filepath)
{
    EngineInstance::LoadWorkingScene(filepath);

    m_selectedObject = GameObject::InvalidReference();

    delete m_cameraController;
    m_cameraController = new CameraController(
        m_renderWindow,
        100.f,
        10.0f);

    SetEditorState(new EditorState());

    return true;
}

void SceneEditor::TerminateEngine()
{
    EngineInstance::TerminateEngine();
}

void SceneEditor::InitializeComponentLibrariesManager()
{
	m_componentLibrariesManager = new EditorComponentLibrariesManager();
}

void SceneEditor::EditorDragAndDropedFile(const blaString& filePath)
{
    FileEntry file = ParseFilePath(filePath);

    if(file.m_extension == ".obj") 
    {
        ImportMesh(file.GetFullPath(), file.m_name);
        return;
    }

    if (file.m_extension == ".blascene")
    {
        LoadWorkingScene(filePath);
        return;
    }

	if(file.m_extension == ".tga")
	{
        Texture2D* texture = TextureImport::LoadTGA(file.m_name, filePath);
        AssetManager::GetSingletonInstance()->SaveTexture(texture);
        AssetManager::GetSingletonInstance()->LoadTexture(file.m_name);
        return;
	}
    if (file.m_extension == ".bmp")
    {
        Texture2D* texture = TextureImport::LoadBMP(file.m_name, filePath);
        AssetManager::GetSingletonInstance()->SaveTexture(texture);
        AssetManager::GetSingletonInstance()->LoadTexture(file.m_name);
        return;
    }
	if(file.m_extension == ".mtl")
	{
        OBJImport::LoadMaterialTemplateLibrary(filePath);
	}
}

void SceneEditor::EditorUpdate()
{
    const InputManager* inputs = InputManager::GetSingletonInstanceRead();

    Ray screenRay;
    if (const BlaGuiRenderWindow* guiRenderWindow = dynamic_cast<const BlaGuiRenderWindow*>(m_guiManager->OpenWindow("Editor Window")))
    {
        screenRay = m_renderer->ScreenToRay(guiRenderWindow->GetMousePointerScreenSpaceCoordinates());

        // ColliderComponent::CollisionContact contactPoint;
        GameObject hoverObject;// = m_scene->PickGameObjectInScene(screenRay, contactPoint);

        auto leftMouseButton = inputs->GetMouseButtonState(BLA_MOUSE_BUTTON_LEFT);
        if (leftMouseButton.IsRisingEdge())
        {
            SetSelectedObject(hoverObject);
        }
    }

    if (m_selectedObject.IsValid())
    {
        if (MeshRendererComponent* meshRenderer = m_selectedObject.GetComponent<MeshRendererComponent>())
        {
            //DebugDraw::DrawArbitraryGeometry(m_selectedObject->GetTransform().GetPosQuat(), m_selectedObject->GetTransform().GetScale(), meshRenderer->m_mesh->m_triangleMesh, blaVec4(BLA::ORANGE, 0.3f));
        }
    }
	
	if(m_bDrawGrid)
	{
        DrawGrid(100, 1.f, WHITE);
	}
	
    DebugDraw::DrawBasis(blaPosQuat::GetIdentity(), 1.f);
}

void SceneEditor::HandleSaveScenePrompt()
{
    EditorSaveSceneState* saveState = static_cast<EditorSaveSceneState*>(m_editorState);

    if (saveState->m_currentFileBrowser)
    {
        blaBool shouldCloseBrowser = false;
        blaString savePath;
        if (saveState->m_currentFileBrowser->GetConfirmedSavePath(savePath))
        {
            SaveWorkingScene(savePath);
            shouldCloseBrowser = true;
        }
        else if (saveState->m_currentFileBrowser->IsBrowsingCancelled())
        {
            shouldCloseBrowser = true;
        }

        if (shouldCloseBrowser)
        {
            m_guiManager->CloseFileBrowser("Save Scene");
            SetEditorState(new EditorState());
        }
    }
    else
    {
        saveState->m_currentFileBrowser = m_guiManager->CreateSaveFilePrompt("Save Scene");
    }
}

void SceneEditor::HandleGuiRequests()
{
    if (m_editorGuiRequests.m_openConsoleRequest)
    {
        m_guiManager->OpenConsole("Console");
        m_editorGuiRequests.m_openConsoleRequest = false;
    }
    if (m_editorGuiRequests.m_openScenGraphGuiRequest)
    {
        if (!m_sceneGraphGui)
        {
            m_sceneGraphGui = new SceneGraphGui();
        }
        m_sceneGraphGui->OpenSceneGraph();
        m_sceneGraphGui->UpdateSceneGraph();

        m_editorGuiRequests.m_openScenGraphGuiRequest = false;
    }
    if (m_editorGuiRequests.m_openComponentInspectorRequest)
    {
        if (!m_componentInspector) m_componentInspector = new GameObjectInspector();

        m_editorGuiRequests.m_openComponentInspectorRequest = false;
    }
}

void SceneEditor::HandleEditorStateChangeRequests()
{
    if (m_editorStateRequests.m_openSceneRequest)
    {
        m_editorStateRequests.m_openSceneRequest = false;
        SetEditorState(new EditorLoadSceneState());
    }

    if (m_editorStateRequests.m_newSceneRequest)
    {
        m_editorStateRequests.m_newSceneRequest = false;
        LoadNewScene();
    }

    if (m_editorStateRequests.m_saveSceneAsRequest)
    {
        m_editorStateRequests.m_saveSceneAsRequest = false;
        SetEditorState(new EditorSaveSceneState());
    }

    if (m_editorStateRequests.m_saveSceneRequest)
    {
        const blaString currentSceneSavePath = m_sceneManager->GetCurrentSceneFilePath();
        m_editorStateRequests.m_saveSceneRequest = false;
        if (currentSceneSavePath.empty())
        {
            SetEditorState(new EditorSaveSceneState());
        }
        else
        {
            SaveWorkingScene(currentSceneSavePath);
        }
    }
}

void SceneEditor::HandleLoadScenePrompt()
{
    EditorLoadSceneState* state = static_cast<EditorLoadSceneState*>(m_editorState);

    if (state->m_currentFileBrowser)
    {
        blaVector<FileEntry> selectedFiles;
        blaBool shouldCloseBrowser = false;
        if (state->m_currentFileBrowser->GetConfirmedSelection(selectedFiles))
        {
            if (!selectedFiles.empty())
            {
                LoadWorkingScene(selectedFiles[0].m_path);
                shouldCloseBrowser = true;
            }
        }
        else if (state->m_currentFileBrowser->IsBrowsingCancelled())
        {
            shouldCloseBrowser = true;
        }

        if (shouldCloseBrowser)
        {
            m_guiManager->CloseFileBrowser("Load Scene File");
            SetEditorState(new EditorState());
        }
    }
    else
    {
        state->m_currentFileBrowser = m_guiManager->CreateOpenFilePrompt("Load Scene File", true);
    }
}

bool SceneEditor::ImportMesh(blaString filepath, blaString name) const
{
    {
        OBJImport objImporter;
        MeshAsset temporaryMeshAsset(name);

        if (!objImporter.ImportMesh(filepath, temporaryMeshAsset.m_triangleMesh, false, true))
        {
            return false;
        }

        m_assetManager->SaveTriangleMesh(&temporaryMeshAsset);
    }

    m_assetManager->LoadTriangleMesh(name);

    GameObject visualizerObject = m_scene->CreateObject(GenerateBlaStringId(name));
    MeshRendererComponent* meshRenderer = visualizerObject.CreateComponent<MeshRendererComponent>();
    //MeshCollider* colliderComp = visualizerObject.CreateComponent<MeshCollider>();

    meshRenderer->m_meshAssetName = name;

    return true;
}

void SceneEditor::SetSelectedObject(GameObject selectedObject)
{
    if (m_selectedObject != selectedObject)
    {
        if (selectedObject.IsValid())
            m_componentInspector->InspectGameObject(selectedObject);
    }
        
    if (selectedObject.IsValid()) 
    {
        m_selectedObject = selectedObject;
    }
}

void SceneEditor::SetObjectParent(GameObject parent, GameObject child)
{
    /*
     * Instead go through a command system sent from the UI.
     * Update Scene graph after each processed command ...
     */
    m_scene->SetGameObjectParent(parent, child);
    m_updatedScene = true;
}

void SceneEditor::DrawGrid(int size, float spacing, const blaVec3& color) const
{
    for (int i = -size / 2; i <= size / 2; i++)
    {
        float iSpacing = i * spacing;
        float sizeSpacing = size * spacing;
        DebugDraw::DrawLine(blaVec3(sizeSpacing / 2, 0, iSpacing), blaVec3(-sizeSpacing / 2, 0, iSpacing), color);
        DebugDraw::DrawLine(blaVec3(iSpacing, 0, sizeSpacing / 2), blaVec3(iSpacing, 0, -sizeSpacing / 2), color);
    }
}

DefineConsoleCommand(void, SelectObject, blaString name)
{
    GameObject obj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->FindObjectByName(name);

    if (obj.IsValid())
    {
        if (SceneEditor* editorSession = dynamic_cast<SceneEditor*>(EngineInstance::GetSingletonInstance()))
        {
            editorSession->SetSelectedObject(obj);
        }
    }
}

DefineConsoleCommand(void, FocusOnObject, blaString name)
{
    GameObject obj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->FindObjectByName(name);

    if (obj.IsValid())
    {
        if (SceneEditor* editorSession = dynamic_cast<SceneEditor*>(EngineInstance::GetSingletonInstance()))
        {
            editorSession->GetCameraController()->SetTransformFocus(obj.GetComponent<TransformComponent>()->GetTransform().GetPosQuat());
        }
    }
}

DefineConsoleCommand(void, SetParent, blaString parentName, blaString childName)
{
    GameObject parentObj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->FindObjectByName(parentName);
    GameObject childObj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->FindObjectByName(childName);

    if (childObj.IsValid())
    {
        if (SceneEditor* editorSession = dynamic_cast<SceneEditor*>(EngineInstance::GetSingletonInstance()))
        {
            editorSession->SetObjectParent(parentObj, childObj); 
        }
    }
}

DefineConsoleCommand(int, SelectScale, blaString name, blaF32 scalex, blaF32 scaley, blaF32 scalez)
{
    GameObject obj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->FindObjectByName(name);

    if (obj.IsValid())
    {
        blaScaledTransform s = obj.GetComponent<TransformComponent>()->GetTransform();
        s.m_scale = blaVec3(scalex, scaley, scalez);
        obj.GetComponent<TransformComponent>()->SetTransform(s);
    }
    return 0;
}

DefineConsoleCommand(void, CreatePointLight, blaString name)
{
    GameObject obj = EngineInstance::GetSingletonInstance()->GetWorkingScene()->CreateObject(GenerateBlaStringId(name));
    obj.CreateComponent(BlaStringId("PointLightComponent"));
}


DefineConsoleCommand(void, AddComponent, blaString objectName, blaString componentName)
{
	GameObject obj(GenerateBlaStringId(objectName));
	if(obj.IsValid()) 
    {
        obj.CreateComponent(GenerateBlaStringId(componentName));
	}
}

DefineConsoleCommand(void, CreateObject, blaString objectName)
{
    Scene* scene = EngineInstance::GetSingletonInstance()->GetWorkingScene();
    scene->CreateObject(GenerateBlaStringId(objectName));
}

DefineConsoleCommand(void, PrintWorkingDirectory)
{
    Console::LogMessage(GetWorkingDir());
}

DefineConsoleCommand(void, PointLight, blaString name)
{
    CreateObject(name);
    AddComponent(name, "PointLightComponent");
}

DefineConsoleCommand(void, CreateMaterial, blaString name, blaString textureName)
{
    Material m(name);
    m.AssignTexture(textureName, "diffuseMap");
    m.AssignTexture("BlankNormal", "normalMap");

    AssetManager::GetSingletonInstance()->SaveMaterial(&m);
    AssetManager::GetSingletonInstance()->LoadMaterial(name);
}

DefineConsoleCommand(void, LoadMTLFile, blaString filename)
{
    OBJImport::LoadMaterialTemplateLibrary(filename);
}

DefineConsoleCommand(void, MakeSponza)
{
    CreateObject("Sponza");
    if (GameObject(BlaStringId("Sponza")).IsValid())
    {
        MeshRendererComponent* r = GameObject(BlaStringId("Sponza")).CreateComponent<MeshRendererComponent>();
        r->m_meshAssetName = "sponza";
        r->m_Render = true;
    }
}