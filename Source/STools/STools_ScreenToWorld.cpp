#include "STools_Precompiled.h"
#include "STools_ScreenToWorld.h"

#include "SG_Camera.h"

namespace Shift
{
	SC_Vector3f STools_ScreenToWorld(const SC_Vector2f& aScreenPos, float aDepth)
	{
		SG_Camera* camera = STools_Editor::GetInstance()->GetCamera();

		SC_Vector3f p;
		p.x = (aScreenPos.x - 0.5f) * 2.0f;
		p.y = -((aScreenPos.y - 0.5f) * 2.0f);
		p.z = aDepth;

		SC_Vector3f wp = camera->Unproject(p);
		return wp;
	}
}