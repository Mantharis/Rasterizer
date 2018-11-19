#pragma once

#include "ModelBase.h"
#include "BaseComp.h"
#include "SceneObject.h"

#include <vector>
#include <memory>

namespace Wolverine
{

	class ModelComp : public IComponent
	{
	private:
		std::vector<std::unique_ptr<IVisual>> m_Visuals;
	
	public:
		ModelComp(std::vector<std::unique_ptr<IVisual>> &&visuals);	
		void render();
	};
}

