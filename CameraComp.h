#pragma once

#include "MathHelper.h"
#include "BaseComp.h"
#include "SceneObject.h"

namespace Wolverine
{

	class CameraComp : public IComponent
	{
	public:
		Mat4x4f GetViewMatrix(SceneObject const &sceneObj) const;
	};

}

