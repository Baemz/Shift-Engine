#pragma once
#include "SR_RenderDefines.h"
#include "SR_TrackedResource.h"

namespace Shift
{
	struct SR_BufferDesc
	{
		uint mySize;
		uint myStructSize;
		uint myBindFlag;

		SR_AccessCPU myCPUAccess;
		SR_AccessGPU myGPUAccess;
		SR_MemoryAccess myMemoryAccess;

		bool myIsWritable : 1;

		SR_BufferDesc()
			: mySize(0)
			, myStructSize(0)
			, myBindFlag(0)
			, myCPUAccess(SR_AccessCPU_None)
			, myGPUAccess(SR_AccessGPU_Read)
			, myMemoryAccess(SR_MemoryAccess_None)
			, myIsWritable(false)

		{}
	};

	class SR_Buffer : public SR_TrackedResource
	{
	public:
		SR_Buffer();
		virtual ~SR_Buffer();

		virtual void Invalidate() = 0;
		
		const SR_BufferDesc& GetProperties() const;

		SR_BufferDesc myDescription;
		char* myData;
	};

#if SR_ENABLE_RAYTRACING
	struct SC_ALIGN(16) SR_RaytracingInstanceData
	{
		char myData[64];
	};
#endif
}