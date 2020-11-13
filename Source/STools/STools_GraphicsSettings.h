#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class STools_GraphicsSettings final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_GraphicsSettings);
	public:
		STools_GraphicsSettings();
		~STools_GraphicsSettings();

		void Render() override;

		const char* GetWindowName() const override { return "Graphics Settings"; }
	private:
		bool mySSAOEnabled;
		float mySSAOIntensity;
	};
}
