#include "SPhysics_Precompiled.h"
#include "SP_Material.h"

namespace Shift
{
	SP_Material::SP_Material()
		: myPxMaterial(nullptr)
	{
	}

	SP_Material::~SP_Material()
	{
		SP_INTERNAL_SAFE_REALEASE(myPxMaterial);
	}
}