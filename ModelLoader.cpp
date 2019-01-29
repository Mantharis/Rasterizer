#include "ModelLoader.h"
#include "Assert.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <vector>
#include <memory>

using namespace std;


using namespace Wolverine;

unique_ptr<ModelComp> ModelLoader::CreateModelComp(SharedMeshData const &meshData)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;

	vector<unique_ptr<IVisual>> visuals;
	visuals.push_back(make_unique<Visual>(meshData));

	return make_unique<ModelComp>(std::move(visuals));
}

unique_ptr<SharedMeshData> ModelLoader::LoadMeshDataFromFile(std::string const &path)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;

	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());
	
	if (!ret)
	{
		return unique_ptr<SharedMeshData>(nullptr);
	}
	
	unique_ptr<SharedMeshData> meshData = make_unique<SharedMeshData>();

	vector<Vec2f> texCoords;
	vector<bool> usedCoordsMap;

	ASSERT(0 == (attrib.vertices.size() % 3), "3 floats per vertex are expected!");

	for (size_t idx = 0; idx< attrib.vertices.size(); idx+=3)
	{
		VertexData vertexData;

		vertexData.pos.set(attrib.vertices[idx + 0], attrib.vertices[idx + 1], attrib.vertices[idx + 2], 1.0f);
		
		meshData->vData.push_back(vertexData);
	}

	for (size_t idx = 0; idx < attrib.texcoords.size(); idx += 2)
	{
		texCoords.push_back(Vec2f(attrib.texcoords[idx], attrib.texcoords[idx + 1]));
	}

	for (auto &shape : shapes)
	{
		for (auto &index : shape.mesh.indices)
		{
			meshData->iData.push_back(index.vertex_index);

			if (index.texcoord_index != -1)
			{
				meshData->vData[index.vertex_index].customData = texCoords[index.texcoord_index];
			}
		}

	}

	ASSERT(0 == (visualData->iData.size() % 3), "3 indices per polygon are expected!");

	return meshData;
}
