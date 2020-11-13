#include "SRender_Precompiled.h"
#include "SR_ShaderCompiler.h"

namespace Shift
{
	SR_ShaderCompiler::SR_ShaderCompiler()
	{
		if (SC_CommandLineManager::HasCommand(L"debugshaders"))
			myDebugShaders = true;

		if (SC_CommandLineManager::HasCommand(L"skipshaderoptimization"))
			mySkipOptimizations = true;

		if (!mySkipOptimizations)
		{
			if (SC_CommandLineManager::HasArgument(L"shaderoptimizationlevel", L"0"))
				myShaderOptimizationLevel = 0;
			else if (SC_CommandLineManager::HasArgument(L"shaderoptimizationlevel", L"1"))
				myShaderOptimizationLevel = 1;
			else if (SC_CommandLineManager::HasArgument(L"shaderoptimizationlevel", L"2"))
				myShaderOptimizationLevel = 2;
			else 
				myShaderOptimizationLevel = 3;
		}
	}

	SR_ShaderCompiler::~SR_ShaderCompiler()
	{
	}
}