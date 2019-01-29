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
		static std::unique_ptr<ModelComp> CreateModelComp(SharedMeshData const &meshData);
		static std::unique_ptr<SharedMeshData> LoadMeshDataFromFile(std::string const &path);

	};

}