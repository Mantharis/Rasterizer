#pragma once
#include <string>
#include <vector>

#include "VertexData.h"
#include "ModelComp.h"
#include "Renderer.h"

namespace Wolverine
{

	class ModelLoader
	{
	public:
		static std::unique_ptr<ModelComp> CreateModelComp(SharedVisualData const &visualData, IRenderer & renderer);
		static std::unique_ptr<SharedVisualData> LoadModelData(std::string const &path);

	};

}