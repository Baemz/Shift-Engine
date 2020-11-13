#pragma once
#include "SR_ShaderCompiler.h"

#if ENABLE_DX12

struct IDxcCompiler3;
struct IDxcUtils;
struct IDxcIncludeHandler;
struct DxcBuffer;
struct IDxcBlob;
struct IDxcBlobUtf16;
namespace Shift
{
	enum SR_ShaderCompiler_Type : uint8
	{
		SR_ShaderCompiler_Type_DXBC,
		SR_ShaderCompiler_Type_DXIL
	};

	struct SR_ShaderCompileInfo
	{
		friend class SR_ShaderCompiler_DX12;
	public:
		SR_ShaderCompileInfo()
			: myDxcBuffer(nullptr)
			, myDXBCBytecode(nullptr)
			, myDXILBytecode(nullptr)
			, myDXILReflection(nullptr)
			, myDXILDebugData(nullptr)
		{}

		SC_GrowingArray<std::wstring> myDefines;

		SR_ShaderType myType;
		const char* myEntryPoint;

		union
		{
			const char* myShaderFile;
			const char* myShaderCode;
		};

	private:
		std::string myCacheName;
		DxcBuffer* myDxcBuffer;

		IDxcBlob* myDXILBytecode;
		IDxcBlob* myDXILReflection;
		IDxcBlob* myDXILDebugData;

		ID3DBlob* myDXBCBytecode;

		union
		{
			const wchar_t* myTargetProfileDXIL;
			const char* myTargetProfileDXBC;
		};
	};

	class SR_ShaderCompiler_DX12 final : public SR_ShaderCompiler
	{
	public:
		SR_ShaderCompiler_DX12(const SR_ShaderCompiler_Type& aCompilerType = SR_ShaderCompiler_Type_DXIL);
		~SR_ShaderCompiler_DX12();

		SC_Ref<SR_ShaderByteCode> CompileShaderFromFile(const char* aShaderFile, const SR_ShaderType aShaderType, std::wstring* aDefines = nullptr, uint aNumDefines = 0) override;
		SC_Ref<SR_ShaderByteCode> CompileShader() override;

	private:
		SC_Ref<SR_ShaderByteCode> CompileInternalDXBC(SR_ShaderCompileInfo& aCompileInfo);
		SC_Ref<SR_ShaderByteCode> CompileInternalDXIL(SR_ShaderCompileInfo& aCompileInfo);
		void InsertHeaderHLSL(std::string& aShaderCode) const;
		std::string LoadHLSL(const char* aShaderFile, std::string& aOutHLSLFile) const;
		void HandleIncludeFile(const std::string& aIncludeLine, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aIncludedFiles, SC_GrowingArray<std::string>& aInsertedSemantics) const;
		bool HandleSemantics(std::ifstream& aShaderFile, const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const;
		bool HandleTextureSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const;
		bool HandleBufferSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const;
		bool HandleSamplerSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const;
		void WriteByteCode(const SR_ShaderCompileInfo& aCompileInfo);
		SC_Ref<SR_ShaderByteCode> FindCompiledByteCode(const char* aCacheName, const SR_ShaderCompiler_Type& aCompilerType);

		IDxcCompiler3* myCompiler;
		IDxcUtils* myUtils;
		IDxcIncludeHandler* myIncludeHandler;

		SR_ShaderCompiler_Type myCompilerType;
	};
}
#endif