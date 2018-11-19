#pragma once

#include "MathHelper.h"
#include <vector>
#include "SceneObject.h"

namespace Wolverine
{


	class Scene
	{
	public:
		bool AddSceneObject(SceneObject &sceneObj);
		bool RemoveSceneObject(SceneObject &sceneObj);
		void SetCamera(SceneObject &camera);

		void Render();
		SceneObject & GetCamera();

	private:
		SceneObject *m_Camera;
		std::vector<SceneObject *> m_SceneObjects;

	};


}

