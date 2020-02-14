#include <windows.h>
#include "System/FileSystem/Files.h"
#include "System/Console.h"
#include "Core/GameComponent.h"
#include "Core/Scene.h"

#include "EditorComponentLibrariesManager.h"

//TODO: Cmake handles that ...
#define BLA_CONFIGURATION "Release"

using namespace BLAengine;

void EditorComponentLibrariesManager::LoadLibraries()
{
	Console::LogMessage("Editor Loading Libraries:");

	blaString libFolder = GetWorkingDir() + "Cooked/Libraries/"+ blaString(BLA_CONFIGURATION);

	blaVector<FileEntry> files;

	GetFilesInDirectory(files, libFolder);

	for(FileEntry& file : files)
	{
        GameComponentRegistry* componentRegistry = GameComponentRegistry::GetSingletonInstance();
        Console* console = Console::GetSingletonInstance();

        blaStringId libraryId = GenerateBlaStringId(file.m_name);

        // We need to inform the component registry and the console that the component declaration and console command / var declarations belong to this specific library
        SetLoadingLibrary(componentRegistry, console, libraryId);

		if(file.m_extension == ".dll")
		{
            if (file.m_name[0] == '_') 
            {
                continue;
            }
			HMODULE load = LoadLibraryA(file.GetFullPath().c_str());
			Console::LogMessage("\tLoading: " + file.m_name + file.m_extension + "| Status: " + (load != NULL ? "Ok" : "Fail"));

            if(!load) // We might have loaded components, we must presume that they are invalid, need to unload everything
            {
                UnloadLibrary(componentRegistry, console, libraryId);
            }
            else 
            {
                m_loadedLibraries.push_back(blaPair<blaStringId, void*>(libraryId, load));
            }
		}
	}
}

void EditorComponentLibrariesManager::UnloadLibraries()
{
    GameComponentRegistry* componentRegistry = GameComponentRegistry::GetSingletonInstance();
    Console* console = Console::GetSingletonInstance();

    for (auto library : m_loadedLibraries)
    {
        UnloadLibrary(componentRegistry, console, library.first);
        FreeLibrary((HMODULE)library.second);
    }
}

void EditorComponentLibrariesManager::ReloadLibraries()
{
    UnloadLibraries();
    LoadLibraries();
}