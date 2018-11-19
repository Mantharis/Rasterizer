#pragma once

#include "ModelBase.h"
#include "Scene.h"
#include "Window.h"

namespace Wolverine
{

	class BasicRenderer : public IRenderer
	{
	public:
		virtual void render(Mat4x4f const &matrix, VisualData &visualData) override;
		BasicRenderer(SdlCanvas &canvas, Scene &scene);

	private:
		SdlCanvas *m_Canvas;
		Scene *m_Scene;
	};

}

