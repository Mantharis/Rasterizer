#include "CameraComp.h"

namespace Wolverine
{
	Mat4x4f CameraComp::getViewMatrix() const
	{
		
		//To transform to view space we need to multiply by inverse -> otherwise its converting from view to world
		//TODO: since the 3x3 matrix is orthogonal we can just transpose it and deal only with the translation separetately
		return gmtl::makeInvert(m_Owner->getMatrix());
	}

}

