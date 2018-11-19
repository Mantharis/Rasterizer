#include "ModelComp.h"
#include "Assert.h"

using namespace Wolverine;
using namespace std;


void ModelComp::render()
{
	for (auto &visual : m_Visuals)
	{
		ASSERT(m_Owner != nullptr, "Owner is null!");
		visual->render(m_Owner->GetMatrix());
	}
}

ModelComp::ModelComp(vector<std::unique_ptr<IVisual>> &&visuals): m_Visuals(move(visuals))
{
}

