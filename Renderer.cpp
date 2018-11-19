#include "Renderer.h"
#include "ctx.h"
#include "Assert.h"
#include "CameraComp.h"
#include "Window.h"
#include <thread>

using namespace Wolverine;
using namespace std;

struct VertexShaderUniforms
{
	Mat4x4f modelMatrix;
	Mat4x4f viewMatrix;
	Mat4x4f projMatrix;
	Mat4x4f viewportMatrix;
};

class VertexShader
{
public:
	VertexData process(VertexData const &in)
	{
		VertexData out;
		out.customData = in.customData;

		//TODO: I can store premultiplied mvp matrix
		out.pos = m_Uniforms.viewportMatrix* m_Uniforms.projMatrix* m_Uniforms.viewMatrix* m_Uniforms.modelMatrix* in.pos;


		Vec4f viewPos = m_Uniforms.viewMatrix* m_Uniforms.modelMatrix* in.pos;

		out.pos /= out.pos[3]; //NDC

		out.pos[2] = viewPos[2]; //TODO, for test only


		return out;
	}

	VertexShaderUniforms m_Uniforms;

};

class FragmentShader
{
public:

	Vec4f process(VertexData const &in) const
	{
		/*
		return Vec4f(
		image( ((int)(in.customData.texCoord[0] * image.width()))% image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 0),
		image(((int)(in.customData.texCoord[0] * image.width())) % image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 1),
		image(((int)(in.customData.texCoord[0] * image.width())) % image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 2),
		in.pos[2]);
		*/


		Vec4f color;


		if ((int)(in.customData.texCoord[0] * 10.0f) % 2 == 0)
		{
			color[0] = 255.0f;
		}

		if ((int)(in.customData.texCoord[1] * 10.0f) % 2 == 0)
		{
			color[1] = 255.0f;
		}

		color[3] = in.pos[2];
		//color.set(in.customData.texCoord[0] *255.0f, 0.0f, 0.0f, in.pos[2]);

		return color;
	}
};

template<typename CANVAS>
struct FrameBuffer
{
public:
	FrameBuffer(CANVAS &canvas, float farPlane) : m_Canvas(&canvas), m_Depth(canvas.GetWidth()*canvas.GetHeight(), farPlane)
	{
	};

	void Clear()
	{
		m_Canvas->Lock();
		m_Canvas->Clear();

		m_Depth.assign(m_Depth.size(), 10000.0f);
	}

	void Present()
	{
		m_Canvas->Unlock();
	}

	void Write(unsigned int x, unsigned int y, float red, float green, float blue, float depth)
	{
		ASSERT(x < m_Canvas->GetWidth() && y < m_Canvas->GetHeight(), "Index is out of range");

		m_Canvas->Write(x, y, (uint8_t)red, (uint8_t)green, (uint8_t)blue);

		m_Depth[y * m_Canvas->GetWidth() + x] = depth;
	}

	float GetDepth(unsigned int x, unsigned int y)
	{
		return m_Depth[y * m_Canvas->GetWidth() + x];
	}

private:
	CANVAS* m_Canvas;

	vector<float> m_Depth;
};

class RasterShader
{
public:
	 void process(size_t x, size_t y, Vec4f const &inputRaster)
	{
		
		if (m_FrameBuffer->GetDepth(x, y) > inputRaster[3])
		{
			m_FrameBuffer->Write(x, y, inputRaster[0], inputRaster[1], inputRaster[2], inputRaster[3]);
		}
		
	}

	FrameBuffer<SdlCanvas> *m_FrameBuffer;
};

BasicRenderer::BasicRenderer( SdlCanvas &canvas, Scene &scene):m_Canvas(&canvas)
{
	m_Scene = &scene;
}

void BasicRenderer::render(Mat4x4f const &modelMatrix, VisualData &visual)
{
	static CTX< VertexShader, FragmentShader, RasterShader, FrameBuffer<SdlCanvas>  > renderCtx;
	static VertexShader vs;
	static FragmentShader fs;
	static FrameBuffer<SdlCanvas> frameBuffer( *m_Canvas, 100.0f);
	static RasterShader  rs;
	
	

	rs.m_FrameBuffer = &frameBuffer;
	
	renderCtx.setResolution( m_Canvas->GetWidth(), m_Canvas->GetHeight());
	renderCtx.setVertexShader(vs);
	renderCtx.setFragmentShader(fs);
	renderCtx.setRasterShader(rs);

	//TODO - cant delete it for every render call!!!
	frameBuffer.Clear();


	Mat4x4f projectionMatrix;
	setProjectionMatrix(1.0f, 100.0f, 1.0f, (float)m_Canvas->GetHeight() / m_Canvas->GetWidth(), projectionMatrix);

	Mat4x4f viewportMatrix;
	setViewportMatrix(m_Canvas->GetWidth(), m_Canvas->GetHeight(), viewportMatrix);

	vs.m_Uniforms.modelMatrix = modelMatrix * visual.matrix;
	vs.m_Uniforms.viewMatrix = m_Scene->GetCamera().FindComponent<CameraComp>()->GetViewMatrix(m_Scene->GetCamera());
	vs.m_Uniforms.projMatrix = projectionMatrix;
	vs.m_Uniforms.viewportMatrix = viewportMatrix;

	
	renderCtx.render(visual.visualData->vData, visual.visualData->iData);

	frameBuffer.Present();
}
