#pragma once

#include "MathHelper.h"
#include <vector>
#include <memory>

namespace Wolverine
{
	class SceneObject;

	class IComponent
	{
	public:
		virtual ~IComponent() {};

		IComponent() :m_Owner(nullptr) {};

	protected:
		SceneObject *m_Owner;

	private:
		friend class SceneObject;
		void SetOwner(SceneObject *owner)
		{
			m_Owner = owner;
		}

	};

	
}

