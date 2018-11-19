#pragma once

#include <gmtl\gmtl.h>

namespace Wolverine
{

	typedef gmtl::Vec<float, 2> Vec2f;
	typedef gmtl::Vec<float, 3> Vec3f;
	typedef gmtl::Vec<float, 4> Vec4f;
	typedef gmtl::Matrix<float, 4, 4> Mat4x4f;
	typedef gmtl::Quat<float> Quatf;
	typedef gmtl::AxisAngle<float> AxisAnglef;


	float deg2Rad(float const angle);

	void setMatrix(Vec3f const &pos, Vec3f const &rotDeg, float scale, Mat4x4f &outMatrix);
	Mat4x4f getMatrix(Vec3f const &pos, Vec3f const &rotDeg, float scale);

	void setIdentityMatrix(Mat4x4f &outMatrix);

	void setViewportMatrix(unsigned int width, unsigned int height, Mat4x4f &outMatrix);

	void setProjectionMatrix(float near, float far, float right, float top, Mat4x4f &outMatrix);

	Vec3f ConvertToVec3(Vec4f const &v4);
	Vec2f ConvertToVec2(Vec4f const &v4);

	template< typename T, typename U> bool isEqual(T const &lOp, T const &rOp, U epsilon)
	{
		return abs(lOp - rOp) < epsilon;
	}
}

