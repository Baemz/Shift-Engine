#pragma once

#if ENABLE_DX12
struct ID3D12RootSignature;
namespace Shift
{
	struct SR_RootSignatureDesc_DX12;

	enum SR_RootTableType
	{
		SR_RootTableType_CBV,
		SR_RootTableType_SRV,
		SR_RootTableType_UAV,
		SR_RootTableType_COUNT
	};

	enum SR_RootSignatureType_DX12
	{
		SR_RootSignatureType_DX12_Graphics,
		SR_RootSignatureType_DX12_Compute,
		SR_RootSignatureType_DX12_Raytracing,
		SR_RootSignatureType_DX12_COUNT
	};

	class SR_RootSignature_DX12 : public SR_Resource
	{
		friend class SR_GraphicsDevice_DX12;
		friend class SR_GraphicsContext_DX12;
	public:
		enum Tables
		{
			LocalTextures,
			LocalConstants,
			LocalBuffers,
			LocalRWTextures,
			LocalRWBuffers,
			GlobalConstants,
			GlobalTextures,
			GlobalBuffers,
			GlobalRWTextures,
			GlobalRWBuffers,
			Tables_Count,

			RootSRV,
			RootCBV,
			COUNT,

			GlobalTables_Start = GlobalConstants,
			GlobalTables_End = GlobalRWBuffers,
			NumGlobalTables = GlobalTables_End - GlobalTables_Start + 1,
		};

		SR_RootSignature_DX12() : myRootSignature(nullptr), myMaxNumDescriptors(0), myType(0) {}
		~SR_RootSignature_DX12();

		void Init(const SR_RootSignatureDesc_DX12& aDesc, const SR_RootSignatureType_DX12& aType);
		uint GetActualTableIndex(const Tables& aTableIndex);
		uint GetNumDescriptors(uint aTableIndex) const { return myTableInfos[aTableIndex].myNumDescriptors; }
		uint GetMaxNumDescriptors() const { return myMaxNumDescriptors; }
		bool IsCompute() const { return myType == SR_RootSignatureType_DX12_Compute; }
		bool IsRaytracing() const { return myType == SR_RootSignatureType_DX12_Raytracing; }
	private:
		struct TableInfo
		{
			uint myTableOffset;
			uint myNumDescriptors;
		};

		TableInfo myTableInfos[Tables::COUNT];
		uint myMaxNumDescriptors;
		ID3D12RootSignature* myRootSignature;
		uint myType;
	};

	struct SR_RootSignatureDesc_DX12
	{
		struct Table
		{
			SR_RootTableType myType;
			uint myNumDescriptors;
		};

		struct RootParam
		{
			RootParam() : myIsUsed(false), myShaderRegister(0), myNum32BitValues(0) {}

			uint myShaderRegister;
			uint myNum32BitValues; // Only for RootCBV 
			bool myIsUsed : 1;
		};

		Table myTableDescs[SR_RootSignature_DX12::Tables::Tables_Count];
		RootParam myRootSRV;
		RootParam myRootCBV;

	};
}
#endif