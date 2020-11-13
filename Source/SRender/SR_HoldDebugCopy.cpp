#include "SRender_Precompiled.h"
#include "SR_HoldDebugCopy.h"

namespace Shift
{
	CHoldDebugCopy::CHoldDebugCopy()
	{
	}
	CHoldDebugCopy::~CHoldDebugCopy()
	{
	}
	SC_GrowingArray<SC_Pair<std::string, SC_Ref<SR_Texture>>>& CHoldDebugCopy::GetStoredTextures()
	{
		return myStoredTextures;
	}
	void CHoldDebugCopy::HoldCopy(SR_TextureBuffer* aTexture, const char* aName)
	{
		SC_UNUSED(aTexture);
		SC_UNUSED(aName);
	}
}