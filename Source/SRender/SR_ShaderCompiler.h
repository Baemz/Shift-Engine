#pragma once
#include "SR_ShaderByteCode.h"

namespace Shift
{
	class SR_ShaderCompiler
	{
	public:
		SR_ShaderCompiler();
		virtual ~SR_ShaderCompiler();

		virtual SC_Ref<SR_ShaderByteCode> CompileShaderFromFile(const char* aShaderFile, const SR_ShaderType aShaderType, std::wstring* aDefines = nullptr, uint aNumDefines = 0) = 0;
		virtual SC_Ref<SR_ShaderByteCode> CompileShader() = 0;

	protected:
		uint8 myShaderOptimizationLevel;
		bool myDebugShaders : 1;
		bool mySkipOptimizations : 1;
	};
}