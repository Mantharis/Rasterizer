#pragma once

#include "MathHelper.h"
#include "BaseComp.h"
#include "VertexData.h"
#include <vector>
#include <memory>

namespace Wolverine
{

	struct SharedVisualData
	{
		std::vector<VertexData> vData;
		std::vector<unsigned int> iData;
	};

	struct VisualData
	{
		SharedVisualData const* visualData;
		Mat4x4f matrix = gmtl::MAT_IDENTITY44F;

		VisualData(SharedVisualData const &sharedData, Mat4x4f const &matrix = gmtl::MAT_IDENTITY44F) : visualData(&sharedData), matrix(matrix)
		{};
	};

	class IRenderer
	{
	public:
		virtual void render(Mat4x4f const &matrix, VisualData &visual) = 0;
		virtual ~IRenderer() {};
	};



	class IVisual
	{
	public:
		virtual void render(Mat4x4f const &matrix) = 0;
		virtual ~IVisual() {};
	};

	class Visual : public IVisual
	{
	public:

		Visual(std::unique_ptr<VisualData> &&visualData, IRenderer &renderer);

		virtual void render(Mat4x4f const &matrix) override
		{
			m_Renderer->render(matrix, *m_VisualData);
		}

	private:
		std::unique_ptr<VisualData> m_VisualData;
		IRenderer	*m_Renderer;
	};

}

