#pragma once
#include "SR_RenderEnums.h"

namespace Shift
{
	struct SR_ShaderDesc
	{
		std::string myFilePath;
		std::string myEntryPoint;
		SR_ShaderType myShaderType;
	};

	class SR_ShaderByteCode : public SR_Resource
	{
	public:
		SR_ShaderByteCode();
		~SR_ShaderByteCode();

		uint GetByteCodeSize() const;
		uint8* GetByteCode() const;

		uint GetHash() const;

	public:
		uint myByteCodeSize;
		uint8* myByteCode;

		void* myReflectionData;

#if !IS_FINAL_BUILD
		std::string myShaderFile;
#endif
	};

}