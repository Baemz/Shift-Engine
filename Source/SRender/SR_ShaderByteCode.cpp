#include "SRender_Precompiled.h"
#include "SR_ShaderByteCode.h"

namespace Shift
{
	SR_ShaderByteCode::SR_ShaderByteCode()
		: myByteCode(nullptr)
		, myByteCodeSize(0)
		, myReflectionData(nullptr)
	{
	}

	SR_ShaderByteCode::~SR_ShaderByteCode()
	{
		delete[] myByteCode;
	}

	uint SR_ShaderByteCode::GetByteCodeSize() const
	{
		return myByteCodeSize;
	}

	uint8* SR_ShaderByteCode::GetByteCode() const
	{
		return myByteCode;
	}

	uint SR_ShaderByteCode::GetHash() const
	{
		return SC_Hash((char*)myByteCode);
	}
}