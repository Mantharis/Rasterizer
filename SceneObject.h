#pragma once

#include "MathHelper.h"
#include <vector>
#include <memory>
#include "BaseComp.h"

namespace Wolverine
{

	class SceneObject
	{
	private:
		Mat4x4f m_Matrix = gmtl::MAT_IDENTITY44F;
		std::vector<std::unique_ptr<IComponent>> m_Components;

	public:
		
		template <typename T> T* FindComponent() const;
		void AddComponent(std::unique_ptr<IComponent> &&comp);

		void SetPos(Vec3f const &pos);
		void SetDir(Vec3f const &dir);

		void Rotate(Vec2f const &rightUpOffset);
		void Scale(float scale);
		void Move(Vec3f const &rightUpDirOffset);

		Vec3f GetRight(bool normalize) const;
		Vec3f GetUp(bool normalize) const;
		Vec3f GetDir(bool normalize) const;
		float GetScale() const;

		Mat4x4f const & GetMatrix() const;
	};

	template <typename T> T* SceneObject::FindComponent() const
	{
		for (auto &it : m_Components)
		{
			T *tComp = dynamic_cast<T*>(&*it);
			if (nullptr != tComp)
			{
				return tComp;
			}
		}

		return nullptr;
	}
}

