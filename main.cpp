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

void initScene(Scene &scene)
{
	static unique_ptr<SharedMeshData> modelData = ModelLoader::LoadMeshDataFromFile("c:/Wolverine/Models/Desk.obj");

	static SceneObject sceneObj;
	sceneObj.setPos(Vec3f(0.0f, 0.0f, -10.0f));
	sceneObj.scale(0.1f);
	sceneObj.addComponent(ModelLoader::CreateModelComp(*modelData));
	scene.addSceneObject(sceneObj);

	static SceneObject sceneObj2;
	sceneObj2.setPos(Vec3f(0.0f, 5.0f, -10.0f));
	sceneObj2.scale(0.1f);
	sceneObj2.addComponent(ModelLoader::CreateModelComp(*modelData));
	scene.addSceneObject(sceneObj2);

	static SceneObject sceneObj3;
	sceneObj3.setPos(Vec3f(0.0f, 10.0f, -10.0f));
	sceneObj3.scale(0.1f);
	sceneObj3.addComponent(ModelLoader::CreateModelComp(*modelData));
	scene.addSceneObject(sceneObj3);
}


void initSceneWithCube(Scene &scene)
{
	static unique_ptr<SharedMeshData> modelData = make_unique<SharedMeshData>();
	modelData->iData = { 
						0,1,2,0,2,3,	//positive Z
						4,7,6,4,6,5,	//negative Z
						6,7,3,6,3,2,	//positive X
						0,4,5,0,5,1,	//negative X
						4,0,3,4,3,7,	//postive Y
						1,5,6,1,6,2,	//negative y
						};


	modelData->vData = {
		{ Vec4f(-1.0f,1.0f, 1.0f, 1.0f), Vec2f(0.0f,0.0f) },
		{ Vec4f(-1.0f,-1.0f, 1.0f, 1.0f), Vec2f(1.0f,0.0f) },
		{ Vec4f(1.0f,-1.0f, 1.0f, 1.0f), Vec2f(1.0f,1.0f) },
		{ Vec4f(1.0f,1.0f, 1.0f, 1.0f), Vec2f(0.0f,1.0f) },

		{ Vec4f(-1.0f,1.0f, -1.0f, 1.0f), Vec2f(0.0f,0.0f) },
		{ Vec4f(-1.0f,-1.0f, -1.0f, 1.0f), Vec2f(1.0f,0.0f) },
		{ Vec4f(1.0f,-1.0f, -1.0f, 1.0f), Vec2f(1.0f,1.0f) },
		{ Vec4f(1.0f,1.0f, -1.0f, 1.0f), Vec2f(0.0f,1.0f) }



	};


	/*
	modelData->vData = {
	{ Vec4f(-11.0f,10.0f, -20.0f, 1.0f), Vec2f(0.0f,0.0f) },
	{ Vec4f(-10.0f,0.0f, -20.0f, 1.0f), Vec2f(0.0f,1.0f) },
	{ Vec4f(10.0f,0.0f, -20.0f, 1.0f), Vec2f(1.0f,1.0f) }

	};
	*/

	static SceneObject sceneObj;
	sceneObj.setPos(Vec3f(0.0f, -5.0f, -10.0f));
	sceneObj.scale(1.0f);
	sceneObj.addComponent(ModelLoader::CreateModelComp(*modelData));
	scene.addSceneObject(sceneObj);
}

void initSceneWithTestingTriangle(Scene &scene)
{
	static unique_ptr<SharedMeshData> modelData = make_unique<SharedMeshData>();
	modelData->iData = { 0,1,2 };

	/*
	modelData->vData = {
		{ Vec4f(0.0f,0.1f, 10.0f, 1.0f), Vec2f(0.0f,0.0f) },
		{ Vec4f(0.0f,0.1f, -10.0f, 1.0f), Vec2f(1.0f,0.0f) },
		{ Vec4f(-1.0f,0.1f, -10.0f, 1.0f), Vec2f(0.0f,1.0f) }

	};
	*/
	
	
	modelData->vData = {
		{ Vec4f(-11.0f,10.0f, -20.0f, 1.0f), Vec2f(0.0f,0.0f) },
		{ Vec4f(-10.0f,0.0f, -20.0f, 1.0f), Vec2f(0.0f,1.0f) },
		{ Vec4f(10.0f,0.0f, -20.0f, 1.0f), Vec2f(1.0f,1.0f) }

	};
	

	static SceneObject sceneObj;
	sceneObj.setPos(Vec3f(0.0f, 0.0f, 0.0f));
	sceneObj.scale(1.0f);
	sceneObj.addComponent(ModelLoader::CreateModelComp(*modelData));
	scene.addSceneObject(sceneObj);
}

int main(int argc, char *argv[])
{
	Scene scene;
	Window window("Wolverine", 1024, 768);
	BasicRenderer renderer(window.getCanvas());

	Mat4x4f projectionMatrix;
	setProjectionMatrix(1.0f, 100.0f, -1.0f /*from camera space its negative right*/, (float)window.getCanvas().getHeight() / window.getCanvas().getWidth(), projectionMatrix);
	renderer.setProjMatrix(projectionMatrix);
	
	Mat4x4f viewportMatrix;
	setViewportMatrix(window.getCanvas().getWidth(), window.getCanvas().getHeight(), viewportMatrix);
	renderer.setViewportMatrix(viewportMatrix);

	//TESTING ONLY
	//initSceneWithTestingTriangle(scene);
	initSceneWithCube(scene);
	//initScene(scene);


	SceneObject camera;
	camera.setDir(Vec3f(0.0f, 0.0f, -1.0f));
	camera.setPos(Vec3f(0.0f, 0.0f, 0.0f));
	camera.addComponent(make_unique<CameraComp>());

	scene.setCamera(camera);

	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_ShowCursor(0);

	float const mouseSensitivity = 0.2f;
	while (true) 
	{
		SDL_Event sdlEvent;

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
					camera.move(Vec3f(0.1f, 0.0f, 0.0f));
					break;

				case SDLK_d:
					camera.move(Vec3f(-0.1f, 0.0f, 0.0f));
					break;

				case SDLK_w:
					camera.move(Vec3f(0.0f, 0.0f, 0.1f));
					break;

				case SDLK_s:
					camera.move(Vec3f(0.0f, 0.0f, -0.1f));
					break;

				case SDLK_q:
					camera.move(Vec3f(0.0f, 0.1f, 0.0f));
					break;

				case SDLK_e:
					camera.move(Vec3f(0.0f, -0.1f, 0.0f));
					break;

				case SDLK_ESCAPE:
					return true;
					break;
				}
				break;

			case SDL_MOUSEMOTION:
				camera.rotate(Vec2f( sdlEvent.motion.yrel, -sdlEvent.motion.xrel) * mouseSensitivity);
				break;

			}
		}
		
		renderer.setViewMatrix(camera.findComponent<CameraComp>()->getViewMatrix());

		renderer.frameBegin();
		
		scene.render(renderer);

		renderer.frameEnd();

		window.update();

	}


	return 0;
}