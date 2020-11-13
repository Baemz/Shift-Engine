#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class STools_Statistics final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_Statistics);
	public:
		STools_Statistics();
		~STools_Statistics();

		void Render() override;

		const char* GetWindowName() const override { return "Statistics"; }
	};

}