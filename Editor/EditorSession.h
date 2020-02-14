#pragma once

#include <StdInclude.h>
#include <EngineInstance.h>
#include <Assets/MeshAsset.h>
#include <EditorGui/GameObjectInspectorGui.h>
#include <EditorGui/SceneGraphGui.h>
#include "GizmoManager.h"

namespace BLAengine
{
    class CameraController;
    class GameObject;
    class EditorState;
    class TriangleMesh;

    class EditorSession : public EngineInstance
    {
    public:
        EditorSession(bool external, bool isFullscreen) :
            EngineInstance(external, isFullscreen),
            m_cameraController(nullptr)
        {}

		bool InitializeEngine(RenderWindow* renderWindow) override;

        void PreEngineUpdate() override;
        void EngineUpdate() override;
        void PostEngineUpdate() override;
        void TerminateEngine() override;

		void InitializeComponentLibrariesManager() override;

        void EditorDragAndDropedFile(const blaString& filePath);

        ~EditorSession() { EngineInstance::~EngineInstance(); };

        GameObject GetSelectedObject() { return m_selectedObject; }

        void SetSelectedObject(GameObject selectedObject);

    private:

        void DrawGrid(int size, float spacing, const blaVec3& color);

        bool LoadNewScene() override;

        bool LoadWorkingScene(blaString filepath) override;

        void SetEditorState(EditorState* state);

        void EditorUpdate();

        /*
        *  Handle State changing things for the editor
        *
        */
        void HandleEditorStateChangeRequests();

        void HandleLoadScenePrompt();

        void HandleSaveScenePrompt();

        void HandleGuiRequests();

        /*
         *  Mesh Import Stuff...
         */
        bool ImportMesh(blaString filepath, blaString name) const;

    private:

        GameObject m_selectedObject;

        GizmoManager m_gizmoManager;
        GameObjectInspector* m_componentInspector = nullptr;
        SceneGraphGui* m_sceneGraphGui = nullptr;
        /*
         * Editor State
         */
        EditorState* m_editorState;
		CameraController* m_cameraController;

        struct EditorStateRequests
        {
            blaBool m_newSceneRequest = false;
            blaBool m_openSceneRequest = false;
            blaBool m_saveSceneRequest = false;
            blaBool m_saveSceneAsRequest = false;
        } m_editorStateRequests;

        struct EditorGuiRequests
        {
            blaBool m_openConsoleRequest = true;
            blaBool m_openScenGraphGuiRequest = true;
            blaBool m_openComponentInspectorRequest = true;
        } m_editorGuiRequests;
    };
};