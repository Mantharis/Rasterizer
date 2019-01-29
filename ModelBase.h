#pragma once

#include "MathHelper.h"
#include "BaseComp.h"
#include "VertexData.h"
#include <vector>
#include <memory>

namespace Wolverine
{

	//Shared between instances of same mesh
	struct SharedMeshData
	{
		std::vector<VertexData> vData;
		std::vector<unsigned int> iData;
	};

	
	class IRenderer
	{
	public:
		virtual void setModelMatrix(Mat4x4f const &matrix) = 0;
		virtual void render( SharedMeshData const &meshData) = 0;
		virtual ~IRenderer() {};
	};



	class IVisual
	{
	public:
		virtual void render(IRenderer &renderer, Mat4x4f const &matrix) = 0;
		virtual ~IVisual() {};
	};

	class Visual : public IVisual
	{
	public:
		Visual::Visual(SharedMeshData const &sharedMeshData) 
			:m_MeshData(&sharedMeshData)
		{
		}

		virtual void render(IRenderer &renderer,Mat4x4f const &parentTransform) override
		{
			renderer.setModelMatrix(parentTransform* matrix);
			renderer.render(*m_MeshData);
		}

	private:
		SharedMeshData const *m_MeshData;
		Mat4x4f matrix = gmtl::MAT_IDENTITY44F;

	};

}

