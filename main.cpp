#include "ctx.h"
#include "lambdaShader.h"
#include "MathHelper.h"
#include "ModelLoader.h"
#include "VertexData.h"
#include "CameraComp.h"
#include "Scene.h"
#include "Renderer.h"
#include "Window.h"

#include <vector>
#include <algorithm>
#include <math.h>
#include <CImg.h>
#include <array>


#include <SDL.h>
#include <stdio.h>

using namespace std;
using namespace Wolverine;




float const nearPlane = 0.1f;
float const farPlane = 10000.0f;

cimg_library::CImg<unsigned char> image("C:/Wolverine/Models/DeskWood_Diffuse.bmp");



//////////////////

float cross(Vec2f const &edge1, Vec2f const &edge2)
{
	return edge1[0] * edge2[1] - edge1[1] * edge2[0];
}

bool caculateBarycentricWeights(float invDoubleTriArea, float doubleTriArea, Vec2f const (&edgesFromTo01_12_20)[3], Vec2f const (&vertex)[3], Vec2f const &point, float(&outWeight)[3])
{
	outWeight[0] = cross(point - vertex[2], edgesFromTo01_12_20[1]);
	outWeight[1] = cross(point - vertex[0], edgesFromTo01_12_20[2]);
	outWeight[2] = cross(point - vertex[1], edgesFromTo01_12_20[0]);

	if (outWeight[0] >= 0.0f &&  outWeight[1] >= 0.0f &&  outWeight[2] >= 0.0f
		//I think this is always true if the first one is true && outWeight[0] + outWeight[1] + outWeight[2] <= doubleTriArea * 1.000001f
		)
	{
		outWeight[0] *= invDoubleTriArea;
		outWeight[1] *= invDoubleTriArea;
		outWeight[2] *= invDoubleTriArea;

		return true;
	}
	else
	{
		return false;
	}
}

void rasterize(Vec3f const &vertex0, Vec3f const &vertex1, Vec3f const &vertex2, vector<Vec3f> &dstBuffer)
{
	Vec2f const edgesFromTo01_12_20[3] =
	{
		Vec2f(vertex1[0] - vertex0[0], vertex1[1] - vertex0[1]),
		Vec2f(vertex2[0] - vertex1[0], vertex2[1] - vertex1[1]),
		Vec2f(vertex0[0] - vertex2[0], vertex0[1] - vertex2[1])
	};

	//in viewport space up and down is switched, thats why '-'
	float const doubleTriangleArea = -cross(edgesFromTo01_12_20[0], edgesFromTo01_12_20[1]);
	float const invDoubleTriangleArea = 1.0f / doubleTriangleArea;


	float weight[3];

	float invZ[3] = { 1.0f / vertex0[2], 1.0f / vertex1[2], 1.0f / vertex2[2] };

	Vec2f p(vertex0[0], vertex0[1]);

	Vec2f vPos[3] = {
		Vec2f(vertex0[0], vertex0[1]),
		Vec2f(vertex1[0], vertex1[1]),
		Vec2f(vertex2[0], vertex2[1])
	};

	const size_t RES_X = 1024;
	const size_t RES_Y = 768;

	size_t idx = 0;
	for (size_t y = 0; y < RES_Y; ++y)
	{
		p[1] = y;

		for (size_t x = 0; x < RES_X; ++x)
		{
			p[0] = x;

			if (caculateBarycentricWeights(invDoubleTriangleArea, doubleTriangleArea, edgesFromTo01_12_20, vPos, p, weight))
			{
				//interpolate values
				float z = 1.0f / (weight[0] * invZ[0] + weight[1] * invZ[1] + weight[2] * invZ[2]);

				dstBuffer[idx] = ((vertex0 * (weight[0] * invZ[0])) + (vertex1 * (weight[1] * invZ[1])) + (vertex2 * (weight[2] * invZ[2]))) * z;
			}

			++idx;
		}
	}
}
//////////////////

using namespace std::chrono;

int main(int argc, char *argv[])
{
	Scene scene;
	Window window("Wolverine", 1024, 768);
	BasicRenderer renderer(window.GetCanvas(), scene);

	/*
	//TESTING ONLY
	unique_ptr<SharedVisualData> modelData = make_unique<SharedVisualData>();
	modelData->iData = { 0,1,2 };
	modelData->vData = { 
		{Vec4f(0.0f,-10.0f, 0.0f, 1.0f), Vec2f(0.0f,0.0f)},
		{Vec4f(10.0f,10.0f, 0.0f, 1.0f), Vec2f(0.0f,0.0f)},
		{Vec4f(-10.0f,10.0f, 0.0f, 1.0f), Vec2f(0.0f,0.0f)}
		 
	};
	*/
	
	unique_ptr<SharedVisualData> modelData = ModelLoader::LoadModelData("c:/Wolverine/Models/Desk.obj");
	 
	SceneObject camera;
	camera.SetDir(Vec3f(0.0f, 0.0f, -1.0f));
	camera.SetPos(Vec3f(0.0f, 0.0f, 0.0f));
	camera.AddComponent(make_unique<CameraComp>());

	SceneObject sceneObj;
	sceneObj.SetPos(Vec3f(0.0f, 0.0f, -10.0f));
	sceneObj.Scale(0.1f);
	sceneObj.AddComponent(ModelLoader::CreateModelComp(*modelData, renderer));

	scene.AddSceneObject(sceneObj);
	scene.SetCamera(camera);

	//transform by model matrix
	//Mat4x4f modelMatrix = getMatrix(Vec3f(0.0f, 0.0f, 50.0f), Vec3f(0.0f, 90.0f, 0.0f), 0.1f);


	////////////////////////////
	//TEST
	/*
	VertexData v[3];
	v[0].pos = Vec4f(-1.0f, -1.0f, 0.0f, 1.0f);
	v[0].customData.texCoord = Vec2f(0, 0);

	v[1].pos = Vec4f(1.0f, -1.0f, 0.0f, 1.0f);
	v[1].customData.texCoord = Vec2f(1.0f, 0);

	v[2].pos = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
	v[2].customData.texCoord = Vec2f(0.5f, 1.0f);


	vData = { v[0], v[1], v[2] };
	iData = { 0,1,2 };
	modelMatrix = getMatrix(Vec3f(0.0f, 0.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f), 5.0f);
	*/
	//END TEST
	////////////////////////////////


	SDL_Event sdlEvent;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_ShowCursor(0);
	

	vector<Vec3f> buffer;
	buffer.resize(1024 * 768);

	high_resolution_clock::time_point start = high_resolution_clock::now();
	size_t fps = 0;

	while (true) 
	{
		//If there's an event to handle
		if (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_QUIT:
				return 0;

			case SDL_KEYDOWN:
				switch (sdlEvent.key.keysym.sym)
				{
				case SDLK_a:
					camera.Move(Vec3f(-0.1f, 0.0f, 0.0f));
					break;

				case SDLK_d:
					camera.Move(Vec3f(0.1f, 0.0f, 0.0f));
					break;

				case SDLK_w:
					camera.Move(Vec3f(0.0f, 0.0f, 0.1f));
					break;

				case SDLK_s:
					camera.Move(Vec3f(0.0f, 0.0f, -0.1f));
					break;

				case SDLK_q:
					camera.Move(Vec3f(0.0f, -0.1f, 0.0f));
					break;

				case SDLK_e:
					camera.Move(Vec3f(0.0f, 0.1f, 0.0f));
					break;

				case SDLK_ESCAPE:
					return true;
					break;
				}
				break;

			case SDL_MOUSEMOTION:
				camera.Rotate(Vec2f(sdlEvent.motion.xrel, -sdlEvent.motion.yrel) * 0.2f);
				break;

			}
		}
		

		//================
		/*
		static Vec3f v0 = { -2000.0f, -2000.0, 10.0 };
		static Vec3f v1 = { 2000.0f, -2000.0, 10.0 };
		static Vec3f v2 = { 0.0f, 2000.0, 10.0 };

		rasterize(v0, v2, v1, buffer);
		++fps;

		high_resolution_clock::time_point now = high_resolution_clock::now();
		auto timeDiff = duration_cast<microseconds>(now - start).count();

		

		static const long oneSecond = 1000000;
		if (timeDiff >= oneSecond)
		{

			cout << fps << " ";
			
			fps = 0;
			start = now;
		}
		*/
		
		
		
		scene.Render();
		window.Update();

	}


	return 0;
}