#pragma once

#include "MathHelper.h"

namespace Wolverine
{


	template<typename TPOS, typename TCUSTOM> struct VertexDataBase
	{
		TPOS pos;
		TCUSTOM customData;
	};

	struct CustomData
	{
		Vec2f texCoord;
		//TODO add new per vertex attributes here

		CustomData operator-(CustomData const &v) const
		{
			CustomData ret = *this;
			ret -= v;
			return ret;
		}

		CustomData& operator-=(CustomData const &v)
		{
			texCoord -= v.texCoord;

			return *this;
		}

		CustomData operator+(CustomData const &v) const
		{
			CustomData ret = *this;
			ret += v;
			return ret;
		}

		CustomData& operator+=(CustomData const &v)
		{
			texCoord += v.texCoord;

			return *this;
		}


		CustomData& operator*=(float const v)
		{
			texCoord *= v;

			return *this;
		}

		CustomData operator*(float const v) const
		{
			CustomData ret = *this;
			ret *= v;
			return ret;
		}

		CustomData& operator/=(float const v)
		{
			texCoord /= v;

			return *this;
		}

		CustomData operator/(float const v) const
		{
			CustomData ret = *this;
			ret /= v;
			return ret;
		}
	};

	typedef VertexDataBase<Vec4f, CustomData> VertexData;

}
