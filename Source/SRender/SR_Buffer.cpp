#include "SRender_Precompiled.h"
#include "SR_Buffer.h"

namespace Shift
{
	SR_Buffer::SR_Buffer()
		: myData(nullptr)
	{
	}

	SR_Buffer::~SR_Buffer()
	{
	}

	const SR_BufferDesc& SR_Buffer::GetProperties() const
	{
		return myDescription;
	}
}