#include "ModelComp.h"
#include "Assert.h"

using namespace Wolverine;
using namespace std;


void ModelComp::render(IRenderer &renderer)
{
	for (auto &visual : m_Visuals)
	{
		ASSERT(m_Owner != nullptr, "Owner is null!");
		visual->render(renderer, m_Owner->getMatrix());
	}
}

ModelComp::ModelComp(vector<std::unique_ptr<IVisual>> &&visuals): m_Visuals(move(visuals))
{
}

