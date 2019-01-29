#include "Scene.h"
#include "ModelComp.h"
#include "CameraComp.h"
#include "Assert.h"
#include <algorithm>
using namespace std;

namespace Wolverine
{
	
	void Scene::setCamera(SceneObject &camera)
	{
		ASSERT(camera.findComponent<CameraComp>(), "No camera component found!");
		m_Camera = &camera;
	}

	bool Scene::addSceneObject(SceneObject &sceneObj)
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

	bool Scene::removeSceneObject(SceneObject &sceneObj)
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

	void Scene::render(IRenderer& renderer)
	{
		for (auto &sceneObj : m_SceneObjects)
		{
			ModelComp *modelComp = sceneObj->findComponent<ModelComp>();
			if (nullptr != modelComp)
			{
				modelComp->render(renderer);
			}
		}
	}
}

