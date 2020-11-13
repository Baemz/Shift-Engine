#pragma once
#include <string>

namespace Shift
{
	struct SR_VertexLayoutElement
	{
		enum EInputClassification
		{
			INPUT_PER_VERTEX_DATA,
			INPUT_PER_INSTANCE_DATA,
		};

		std::string mySemanticName;
		uint mySemanticIndex;
		SR_Format myFormat;
		uint myInputSlot;
		uint myAlignedByteOffset;
		EInputClassification myInputSlotClass;
		uint myInstanceDataStepRate;
	};

	class SR_VertexLayout
	{
		friend class SR_GraphicsDevice_DX12;

	public:
		SR_VertexLayout();
		~SR_VertexLayout();

	private:
		SC_GrowingArray<SR_VertexLayoutElement> myElements;
	};

}