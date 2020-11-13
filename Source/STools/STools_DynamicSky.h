#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class SG_DynamicSky;
	class STools_DynamicSky final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_DynamicSky);
	public:
		STools_DynamicSky() {}
		STools_DynamicSky(SG_DynamicSky* aSky);
		~STools_DynamicSky();

		void Render() override;
		const char* GetWindowName() const override { return "Dynamic Sky"; }
	private:
		SG_DynamicSky* myDynamicSky;
		bool myOverridePCFFilter;
	};
}

