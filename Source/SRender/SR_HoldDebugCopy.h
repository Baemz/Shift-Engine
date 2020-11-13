#pragma once

namespace Shift
{
	class CHoldDebugCopy
	{
	public:
		CHoldDebugCopy();
		~CHoldDebugCopy();

		SC_GrowingArray<SC_Pair<std::string, SC_Ref<SR_Texture>>>& GetStoredTextures();

		void HoldCopy(SR_TextureBuffer* aTexture, const char* aName);

	private:
		SC_GrowingArray<SC_Pair<std::string, SC_Ref<SR_Texture>>> myStoredTextures;
	};

}