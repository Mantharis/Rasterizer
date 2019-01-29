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
		
		template <typename T> T* findComponent() const;
		void addComponent(std::unique_ptr<IComponent> &&comp);

		void setPos(Vec3f const &pos);
		void setDir(Vec3f const &dir);

		void rotate(Vec2f const &rightUpOffset);
		void scale(float scale);
		void move(Vec3f const &rightUpDirOffset);

		Vec3f getRight(bool normalize) const;
		Vec3f getUp(bool normalize) const;
		Vec3f getDir(bool normalize) const;
		float getScale() const;

		Mat4x4f const & getMatrix() const;
	};

	template <typename T> T* SceneObject::findComponent() const
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

