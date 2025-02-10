#include "Scene/SceneManager.h"

namespace LE
{
	SceneManager gSceneManager;

	void SceneManager::LoadCurrentScene()
	{
		CurrentScene.Initialize();
	}

	void SceneManager::UnloadCurrentScene()
	{
		CurrentScene.Shutdown();
	}

	void SceneManager::Update()
	{
		CurrentScene.Update();
	}

	void SceneManager::PostUpdate()
	{
		CurrentScene.PostUpdate();
	}
}