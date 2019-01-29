#pragma once

#include "ModelBase.h"
#include "Scene.h"
#include "Window.h"
#include "ctx.h"
#include <vector>

namespace Wolverine
{

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
			//TEST---------------
			/*
			VertexData tst;

			VertexData nearCenter = in;
			//nearCenter.pos.set(1.0f, 0.0f, -3.0f, 1.0f);

			tst.pos =  (m_Uniforms.modelMatrix* nearCenter.pos);

			tst.pos = (m_Uniforms.viewMatrix* (m_Uniforms.modelMatrix* nearCenter.pos));
			tst.pos = m_Uniforms.projMatrix* (m_Uniforms.viewMatrix* (m_Uniforms.modelMatrix* nearCenter.pos));
			*/

			VertexData out = in;

			//Performance: I could store premultiplied mvp matrix here
			out.pos = m_Uniforms.projMatrix* (m_Uniforms.viewMatrix* (m_Uniforms.modelMatrix* in.pos));
			
			return out;
		}

		VertexShaderUniforms m_Uniforms;

	};

	class FragmentShader
	{
	public:

		void process(Pack4f &fragmentTexCoordU, Pack4f &fragmentTexCoordV, Pack4f const &fragmentDepth, Vec3b color[4], float depth[4])
		{
			//TODO GetDataPtr
			if ((int)(fragmentTexCoordU * Pack4f(10.0f)).GetDataPtr()[0] % 2 == 0)
			{
				color[0][0] = 255;
				color[1][0] = 255;
				color[2][0] = 255;
				color[3][0] = 255;
			}

			if ((int)(fragmentTexCoordV * Pack4f(10.0f)).GetDataPtr()[0] % 2 == 0)
			{
				color[0][1] = 255;
				color[1][1] = 255;
				color[2][1] = 255;
				color[3][1] = 255;
			}

			depth[0] = fragmentDepth.GetDataPtr()[0];
			depth[1] = fragmentDepth.GetDataPtr()[1];
			depth[2] = fragmentDepth.GetDataPtr()[2];
			depth[3] = fragmentDepth.GetDataPtr()[3];

		}
		//TODO Vec4b cost the same as Vec3b, use that
		void process(FragmentData const &fragment, Vec3b &color, float &depth) const
		{
			/*
			return Vec4f(
			image( ((int)(in.customData.texCoord[0] * image.width()))% image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 0),
			image(((int)(in.customData.texCoord[0] * image.width())) % image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 1),
			image(((int)(in.customData.texCoord[0] * image.width())) % image.width(), image.height() - ((int)(in.customData.texCoord[1] * image.height())) % image.height(), 0, 2),
			in.pos[2]);
			*/


		
			if ((int)(fragment[0] * 10.0f) % 2 == 0)
			{
				color[0] = 255;
			}

			if ((int)(fragment[1] * 10.0f) % 2 == 0)
			{
				color[1] = 255;
			}
			
			depth = fragment[2];
		}
	};

	template<typename CANVAS>
	struct FrameBuffer
	{
	public:
		FrameBuffer(CANVAS &canvas) : m_Canvas(&canvas), m_Depth(canvas.getWidth()*canvas.getHeight(), 1.0f)
		{
		};

		void frameBegin()
		{
			m_Canvas->lock();
			m_Canvas->clear();
			m_Depth.assign(m_Depth.size(), 1.0f);
		}

		void frameEnd()
		{
			m_Canvas->unlock();
		}

		void write(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue, float depth)
		{
			if (x >= m_Canvas->getWidth() || y >= m_Canvas->getHeight())
			{
				return;
			}

			//ASSERT(x < m_Canvas->GetWidth() && y < m_Canvas->GetHeight(), "Index is out of range");

			m_Canvas->write(x, y, red, green, blue);

			m_Depth[y * m_Canvas->getWidth() + x] = depth;
		}

		float getDepth(size_t x, size_t y)
		{
			return m_Depth[y * m_Canvas->getWidth() + x];
		}

	private:
		std::vector<float> m_Depth;
		CANVAS* m_Canvas;
	};

	//Values recieved from FragmentShader goes into RasterShader who is responsible to write them into Framebuffer
	//Can for example do blending between input and value in framebuffer
	class RasterShader
	{
	public:
		void process(size_t x, size_t y, Vec3b const &inputRaster, float depth)
		{
			if (m_FrameBuffer->getDepth(x, y) > depth)
			{
				m_FrameBuffer->write(x, y, inputRaster[0], inputRaster[1], inputRaster[2], depth);
			}
		}

		FrameBuffer<SdlCanvas> *m_FrameBuffer;
	};

	class BasicRenderer : public IRenderer
	{
	public:
		BasicRenderer(SdlCanvas &canvas);

		void frameBegin();
		void frameEnd();

		virtual void render(SharedMeshData const &meshData) override;
		virtual void setModelMatrix(Mat4x4f const &modelMatrix) override;

		void setViewportMatrix(Mat4x4f const &viewportMatrix);
		void setProjMatrix(Mat4x4f const &projMatrix);
		void setViewMatrix(Mat4x4f const &viewMatrix);

	private:
		CTX< VertexShader, FragmentShader, RasterShader, FrameBuffer<SdlCanvas>  > m_RenderCtx;
		VertexShader m_VertexShader;
		FragmentShader m_FragmentShader;
		RasterShader m_RasterShader;
		FrameBuffer<SdlCanvas> m_FrameBuffer;

		Mat4x4f m_ModelMatrix;
		Mat4x4f m_ViewMatrix;
		Mat4x4f m_ProjMatrix;
		Mat4x4f m_ViewportMatrix;
	};

}

