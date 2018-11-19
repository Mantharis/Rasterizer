#include "SceneObject.h"

namespace Wolverine
{
	
	Mat4x4f const & SceneObject::GetMatrix() const
	{
		return m_Matrix;
	}

	void SceneObject::AddComponent(std::unique_ptr<IComponent> &&comp)
	{
		comp->SetOwner(this);
		m_Components.push_back(std::move(comp));
	}

	void SceneObject::SetPos(Vec3f const &pos)
	{
		m_Matrix(0, 3) = pos[0];
		m_Matrix(1, 3) = pos[1];
		m_Matrix(2, 3) = pos[2];
	}

	void SceneObject::SetDir(Vec3f const &dir)
	{
		float lenRight = gmtl::length(GetRight(false));
		Vec3f right = gmtl::makeCross(dir, Vec3f(0.0f, 1.0f, 0.0f));
		gmtl::normalize(right);
		right *= lenRight; //maintain original right vector lenght

		float lenUp = gmtl::length(GetUp(false));
		Vec3f up = gmtl::makeCross(right, dir);
		gmtl::normalize(up);
		up *= lenUp;

		m_Matrix.set(
			right[0], up[0], dir[0], m_Matrix(0, 3),
			right[1], up[1], dir[1], m_Matrix(1, 3),
			right[2], up[2], dir[2], m_Matrix(2, 3),
			0, 0, 0, 1.0f);
	}

	void SceneObject::Rotate(Vec2f const &rightUpOffset)
	{
		Vec3f dir = GetDir(false);

		Quatf rotY;
		gmtl::setRot(rotY, gmtl::AxisAngle<float>(deg2Rad(-rightUpOffset[0]), Vec3f(0.0f, 1.0f, 0.0f)));

		Quatf rotX;
		Vec3f right = GetRight(false);


		gmtl::setRot(rotX, gmtl::AxisAngle<float>(deg2Rad(rightUpOffset[1]), right));

		Quatf rotXY = rotX * rotY;
		dir *= rotXY;

		SetDir(dir);
	}

	void SceneObject::Scale(float scale)
	{
		for (int y = 0; y < 3; ++y)
		{
			for (int x = 0; x<3; ++x)
			{
				m_Matrix(y,x) *= scale;
			}
		}
		
	}

	Vec3f SceneObject::GetRight(bool normalize) const
	{
		Vec3f right = Vec3f(m_Matrix(0, 0), m_Matrix(1, 0), m_Matrix(2, 0));
		if (normalize)
		{
			gmtl::normalize(right);
		}

		return right;
	}

	Vec3f SceneObject::GetUp(bool normalize) const
	{
		Vec3f up = Vec3f(m_Matrix(0, 1), m_Matrix(1, 1), m_Matrix(2, 1));
		if (normalize)
		{
			gmtl::normalize(up);
		}

		return up;
	}

	Vec3f SceneObject::GetDir(bool normalize) const
	{
		Vec3f dir = Vec3f(m_Matrix(0, 2), m_Matrix(1, 2), m_Matrix(2, 2));
		if (normalize)
		{
			gmtl::normalize(dir);
		}

		return dir;
	}


	void SceneObject::Move(Vec3f const &rightUpDirOffset)
	{
		Vec3f offset = (GetRight(true) * rightUpDirOffset[0]) + (GetUp(true) * rightUpDirOffset[1]) + (GetDir(true) * rightUpDirOffset[2]);

		m_Matrix(0, 3) += offset[0];
		m_Matrix(1, 3) += offset[1];
		m_Matrix(2, 3) += offset[2];
	}


}

