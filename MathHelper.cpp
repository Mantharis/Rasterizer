#include "MathHelper.h"

namespace Wolverine
{

	float deg2Rad(float const angle)
	{
		const float PI = 3.1415927;
		const float radToDegMultiplier = PI/ 180.0f;

		return radToDegMultiplier * angle;
	}

	void setMatrix(Vec3f const &pos, Vec3f const &rotDeg, float scale, Mat4x4f &outMatrix)
	{
		gmtl::identity(outMatrix);
		gmtl::setRot(outMatrix, gmtl::EulerAngleXYZf(deg2Rad(rotDeg[0]) , deg2Rad(rotDeg[1]), deg2Rad(rotDeg[2])));
		gmtl::mult(outMatrix, scale);
		outMatrix.mData[15] = 1.0f; //do not scale last element

		gmtl::setTrans(outMatrix, pos);
	}

	void setIdentityMatrix(Mat4x4f &outMatrix)
	{
		gmtl::identity(outMatrix);
	}

	Mat4x4f getMatrix(Vec3f const &pos, Vec3f const &rotDeg, float scale)
	{
		Mat4x4f res;
		setMatrix(pos, rotDeg, scale, res);

		return res;
	}

	void setViewportMatrix(unsigned int width, unsigned int height, Mat4x4f &outMatrix)
	{
		outMatrix.set(	0.5f * width, 0.0f, 0.0f, 0.5f * width,
						0.0f, -0.5f * height, 0.0f, 0.5f * height,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
	}

	void setProjectionMatrix(float near, float far, float right, float top, Mat4x4f &outMatrix)
	{
		outMatrix.set(	near/right, 0.0f, 0.0f, 0.0f,
						0.0f, near / top, 0.0f, 0.0f,
						//0.0f, 0.0f, 1.0f, 1.0f, //TODO after dividing by w we always get z=1 ...that makes z test useless, fix that! 
						0.0f, 0.0f, (far+near)/(far-near), (-2.0f * near * far)/(far-near), //TODO this line is different than whats on internet, I will think about it later
						0.0f, 0.0f, 1.0f, 0.0f);
	}

	Vec3f ConvertToVec3(Vec4f const &v4)
	{
		return Vec3f(v4[0], v4[1], v4[2]);
	}

	Vec2f ConvertToVec2(Vec4f const &v4)
	{
		return Vec2f(v4[0], v4[1]);
	}
}

