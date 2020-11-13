#pragma once
#include "SR_GraphicsResource.h"
#include "SR_Buffer.h"

namespace Shift
{
	enum SR_BufferViewType
	{
		SR_BufferViewType_None,
		SR_BufferViewType_Bytes,
		SR_BufferViewType_Structured,
		SR_BufferViewType_Append_Consume,
		SR_BufferViewType_RaytracingScene,

		SR_BufferViewType_Count,
	};

	struct SR_BufferViewDesc
	{
		SR_BufferViewType myType;
		SR_Format myFormat;
		uint myFirstElement; 
		uint myNumElements;
		bool myIsShaderWritable;

		SR_BufferViewDesc()
			: myType(SR_BufferViewType_None)
			, myFormat(SR_Format_Unknown)
			, myNumElements(0)
			, myFirstElement(0)
			, myIsShaderWritable(false)
		{}
	};

	class SR_BufferView : public SR_Resource
	{
		friend class SR_GraphicsDevice_DX12;
	public:
		SR_BufferView();
		~SR_BufferView();

		SR_Buffer* GetBuffer() const { return myBuffer; }

		SR_BufferViewDesc myDescription;
	private:
		SC_Ref<SR_Buffer> myBuffer;
	};
}