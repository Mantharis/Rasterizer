#include "Modelbase.h"

using namespace Wolverine;
using namespace std;

Visual::Visual(std::unique_ptr<VisualData> &&visualData, IRenderer &renderer):m_VisualData(move(visualData)), m_Renderer(&renderer)
{
}