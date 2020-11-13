#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class SR_Texture;
	class STools_TextureViewer final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_TextureViewer);
	public:
		STools_TextureViewer();
		~STools_TextureViewer();

		void Render() override;

		const char* GetWindowName() const override { return "Texture Viewer"; }

	private:
		SR_Texture* mySelectedTexture;
	};
}
