#include "Renderer.h"
#include "Assert.h"
#include "CameraComp.h"
#include "Window.h"
#include <thread>

using namespace Wolverine;
using namespace std;

BasicRenderer::BasicRenderer( SdlCanvas &canvas): m_FrameBuffer(canvas)
{
	m_RasterShader.m_FrameBuffer = &m_FrameBuffer;

	m_RenderCtx.setResolution( canvas.getWidth(), canvas.getHeight());
	m_RenderCtx.setVertexShader(m_VertexShader);
	m_RenderCtx.setFragmentShader(m_FragmentShader);
	m_RenderCtx.setRasterShader(m_RasterShader);
}

void BasicRenderer::setModelMatrix(Mat4x4f const &modelMatrix)
{
	m_ModelMatrix = modelMatrix;
}

void BasicRenderer::setViewMatrix(Mat4x4f const &viewMatrix)
{
	m_ViewMatrix = viewMatrix;
}

void BasicRenderer::setProjMatrix(Mat4x4f const &projMatrix)
{
	m_ProjMatrix = projMatrix;
}

void BasicRenderer::setViewportMatrix(Mat4x4f const &viewportMatrix)
{
	m_ViewportMatrix = viewportMatrix;
}

void BasicRenderer::frameBegin()
{
	m_FrameBuffer.frameBegin();
}

void BasicRenderer::frameEnd()
{
	m_FrameBuffer.frameEnd();
}

void BasicRenderer::render(SharedMeshData const &meshData)
{
	m_VertexShader.m_Uniforms.modelMatrix = m_ModelMatrix;
	m_VertexShader.m_Uniforms.viewMatrix = m_ViewMatrix;
	m_VertexShader.m_Uniforms.projMatrix = m_ProjMatrix;
	m_VertexShader.m_Uniforms.viewportMatrix = m_ViewportMatrix;

	m_RenderCtx.render(meshData.vData, meshData.iData);
}
