#include "Scene.h"
#include "ModelComp.h"
#include "CameraComp.h"
#include "Assert.h"
#include <algorithm>
using namespace std;

namespace Wolverine
{
	
	SceneObject & Scene::GetCamera()
	{
		return *m_Camera;
	}

	void Scene::SetCamera(SceneObject &camera)
	{
		ASSERT(camera.FindComponent<CameraComp>(), "No camera component found!");
		m_Camera = &camera;
	}

	bool Scene::AddSceneObject(SceneObject &sceneObj)
	{
		auto it = find(m_SceneObjects.begin(), m_SceneObjects.end(), &sceneObj);
		if (m_SceneObjects.end() == it)
		{
			m_SceneObjects.push_back(&sceneObj);
			return true;
		}
		else
		{
			return false; //already in scene
		}
	}

	bool Scene::RemoveSceneObject(SceneObject &sceneObj)
	{
		auto it = find(m_SceneObjects.begin(), m_SceneObjects.end(), &sceneObj);
		if (m_SceneObjects.end() != it)
		{
			swap(*m_SceneObjects.rbegin(), *it);
			m_SceneObjects.pop_back();
			return true;
		}
		else
		{
			return false;
		}
	}

	void Scene::Render()
	{
		for (auto &sceneObj : m_SceneObjects)
		{
			ModelComp *modelComp = sceneObj->FindComponent<ModelComp>();
			if (nullptr != modelComp)
			{
				modelComp->render();
			}
		}
	}
}

