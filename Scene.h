#pragma once

#include "MathHelper.h"
#include <vector>
#include "SceneObject.h"
#include "Renderer.h"

namespace Wolverine
{


	class Scene
	{
	public:
		bool addSceneObject(SceneObject &sceneObj);
		bool removeSceneObject(SceneObject &sceneObj);
		void setCamera(SceneObject &camera);

		void render(IRenderer &renderer);

	private:
		SceneObject *m_Camera;
		std::vector<SceneObject *> m_SceneObjects;

	};


}

