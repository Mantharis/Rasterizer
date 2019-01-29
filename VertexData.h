#pragma once

#include "MathHelper.h"

namespace Wolverine
{


	template<typename TPOS, typename TCUSTOM> struct VertexDataBase
	{
		TPOS pos;
		TCUSTOM customData;
	};

	//pos.xyzw, texCoord.xy
	typedef VertexDataBase<Vec4f, Vec2f> VertexData;

	//[0] = TexCoordX, [1] = TexCoordY, [2] = depth
	typedef Vec3f FragmentData;
}
