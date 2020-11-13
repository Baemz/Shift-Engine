#include "SRender_Precompiled.h"
#include "SR_ShaderCompiler_DX12.h"

#if ENABLE_DX12
#include "dxc/dxcapi.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/dxcapi.use.h"

#include <d3d12shader.h>
#include <fstream>
#include <sstream>

#pragma comment(lib,"dxcompiler.lib")

namespace Shift
{
	static uint locCompiledShadersCount = 0;

	std::string locMakeShaderCacheName(const char* aHLSLCode, const char* aTargetProfile)
	{
		uint hlslHash = SC_Hash(aHLSLCode);
		uint targetHash = SC_Hash(aTargetProfile);

		std::stringstream hashStream;
		hashStream << std::hex << hlslHash << std::hex << targetHash;

		return hashStream.str();
	}

	std::string locMakeShaderCacheName(const char* aHLSLCode, const wchar_t* aTargetProfile)
	{
		std::wstring targetW(aTargetProfile);
		std::string target = ToString(targetW);
		return locMakeShaderCacheName(aHLSLCode, (const char*)target.c_str());
	}

	std::string RetrieveHashFromBlob(IDxcBlob* pHashBlob) 
	{
		assert(pHashBlob != nullptr);
		assert(pHashBlob->GetBufferSize() == sizeof(DxcShaderHash));

		const hlsl::DxilShaderHash* pShaderHash = (hlsl::DxilShaderHash*)pHashBlob->GetBufferPointer();
		std::stringstream os;

		for (int i = 0; i < 16; ++i)
			os << GetFormattedString("%.2x", pShaderHash->Digest[i]);
		return os.str();
	}

	ID3D12ShaderReflection* locCreateReflectionFromBlob(IDxcBlob* aBlob)
	{
		static dxc::DxcDllSupport ourDLLSupport;
		if (!ourDLLSupport.IsEnabled())
			ourDLLSupport.Initialize();

		IDxcContainerReflection* reflection = nullptr;
		UINT32 shaderIdx;
		ourDLLSupport.CreateInstance(CLSID_DxcContainerReflection, &reflection);
		if (FAILED(reflection->Load(aBlob)))
		{
			SC_ERROR_LOG("Failed to load shader blob");
			return nullptr;
		}
		if (FAILED(reflection->FindFirstPartKind(hlsl::DFCC_DXIL, &shaderIdx)))
		{
			SC_ERROR_LOG("Failed to find shader input signature");
			return nullptr;
		}
		ID3D12ShaderReflection* shaderReflection;
		if (FAILED(reflection->GetPartReflection(shaderIdx, SR_IID_PPV_ARGS(&shaderReflection))))
		{
			SC_ERROR_LOG("Failed to get shader input signature");
			return nullptr;
		}
		reflection->Release();
		return shaderReflection;
	}

	static const wchar_t* locGetDXILTarget(const SR_ShaderType aShaderType)
	{
		switch (aShaderType)
		{
		case SR_ShaderType_Vertex:
			return L"vs_6_4";
		case SR_ShaderType_Pixel:
			return L"ps_6_4";

#if SR_ENABLE_TESSELATION_SHADERS
		case SR_ShaderType_Hull:
			return L"hs_6_4";
		case SR_ShaderType_Domain:
			return L"ds_6_4";
#endif

#if SR_ENABLE_GEOMETRY_SHADERS
		case SR_ShaderType_Geometry:
			return L"gs_6_4";
#endif
		case SR_ShaderType_Compute:
			return L"cs_6_4";
#if SR_ENABLE_MESH_SHADERS
		case SR_ShaderType_Mesh:
			return L"ms_6_5";
		case SR_ShaderType_Amplification:
			return L"as_6_5";
#endif

#if SR_ENABLE_RAYTRACING
		case SR_ShaderType_Raytracing:
			return L"lib_6_4";
#endif
		default:
			assert(false);
			return L"Unknown target";
		}
	}

	static const char* locGetDXBCTarget(const SR_ShaderType aShaderType)
	{
		switch (aShaderType)
		{
		case SR_ShaderType_Vertex:
			return "vs_5_1";
		case SR_ShaderType_Pixel:
			return "ps_5_1";

#if SR_ENABLE_TESSELATION_SHADERS
		case SR_ShaderType_Hull:
			return "hs_5_1";
		case SR_ShaderType_Domain:
			return "ds_5_1";
#endif

#if SR_ENABLE_GEOMETRY_SHADERS
		case SR_ShaderType_Geometry:
			return "gs_5_1";
#endif
		case SR_ShaderType_Compute:
			return "cs_5_1";

		default:
			assert(false);
			return "Unknown target";
		}
	}

	template<class T>
	uint locGetRefIndex(const std::string& aLineOfCode)
	{
		const int* values = T::Values();
		const char* const* refs = T::Names();
		const uint count = T::myCount;

		for (uint i = 0; i < count; ++i)
		{
			if (aLineOfCode.find(refs[i]) != aLineOfCode.npos)
				return values[i];
		}

		assert(false);
		return SC_UINT_MAX;
	}

	template<class T>
	uint locGetLocalRefIndex(const std::string& aLineOfCode, const char* aType)
	{
		const int* values = T::Values();
		const char* const* cbRefs = T::Names();
		const uint count = T::myCount;

		std::string registerString;
		registerString.reserve(128);
		registerString = T::TypeName;
		registerString += "_Local";

		size_t typeLength = strlen(aType);
		size_t firstIndex = aLineOfCode.rfind(aType) + typeLength;
		size_t semicolonIndex = aLineOfCode.rfind(';');
		size_t lastIndex = (semicolonIndex != aLineOfCode.npos) ? semicolonIndex : aLineOfCode.length();
		std::string temp = aLineOfCode.substr(firstIndex, lastIndex - firstIndex);
		registerString += temp;

		for (uint i = 0; i < count; ++i)
		{
			if (registerString == cbRefs[i])
				return values[i];
		}

		assert(false);
		return SC_UINT_MAX;
	}

	const char* locGetSamplerType(uint aRegisterSlot)
	{
		if (aRegisterSlot >= SR_SamplerRef::SR_SamplerRef_CmpGreater_Linear_Clamp)
			return "SamplerComparisonState";

		return "SamplerState";
	}

	const char* locGetTextureType(const std::string& aLineOfCode)
	{		
		if (aLineOfCode.find("Texture2DArray") != aLineOfCode.npos)
			return "Texture2DArray";
		else if (aLineOfCode.find("TextureCubeArray") != aLineOfCode.npos)
			return "TextureCubeArray";
		else if (aLineOfCode.find("Texture2D") != aLineOfCode.npos)
			return "Texture2D";
		else if (aLineOfCode.find("Texture3D") != aLineOfCode.npos)
			return "Texture3D";
		else if (aLineOfCode.find("TextureCube") != aLineOfCode.npos)
			return "TextureCube";

		return "__Unknown__";
	}

	const char* locGetBufferType(const std::string& aLineOfCode)
	{
		if (aLineOfCode.find("AccelerationStructure") != aLineOfCode.npos)
			return "RaytracingAccelerationStructure";
		else if (aLineOfCode.find("StructuredBuffer") != aLineOfCode.npos)
			return "StructuredBuffer";
		else if (aLineOfCode.find("ByteBuffer") != aLineOfCode.npos)
			return "ByteAddressBuffer";
		else if (aLineOfCode.find("Buffer") != aLineOfCode.npos)
			return "Buffer";

		return "__Unknown__";
	}

	std::string locGetDataType(const std::string& aLineOfCode)
	{
		size_t startDelimiter = aLineOfCode.find('<');
		size_t closingDelimiter = aLineOfCode.find('>');

		if (startDelimiter == aLineOfCode.npos || closingDelimiter == aLineOfCode.npos)
			return "";

		return aLineOfCode.substr(startDelimiter + 1, (closingDelimiter - startDelimiter) - 1);
	}
	/*static uint locGetTextureRefIndex(const std::string& aLineOfCode)
	{
		const int* values = SR_TextureRef::Values();
		const char* const* textureRefs = SR_TextureRef::Names();
		const uint count = SR_TextureRef::myCount;

		for (uint i = 0; i < count; ++i)
		{
			if (aLineOfCode.find(textureRefs[i]) != aLineOfCode.npos)
				return values[i];
		}

		assert(false);
		return SC_UINT_MAX;
	}

	static uint locGetSamplerRefIndex(const std::string& aLineOfCode)
	{
		const int* values = SR_SamplerRef::Values();
		const char* const* samplerRefs = SR_SamplerRef::Names();
		const uint count = SR_SamplerRef::myCount;

		for (uint i = 0; i < count; ++i)
		{
			if (aLineOfCode.find(samplerRefs[i]) != aLineOfCode.npos)
				return values[i];
		}

		assert(false);
		return SC_UINT_MAX;
	}*/



	SR_ShaderCompiler_DX12::SR_ShaderCompiler_DX12(const SR_ShaderCompiler_Type& /*aCompilerType*/)
	{
		if (SC_CommandLineManager::HasCommand(L"usedxbc") || SC_CommandLineManager::HasCommand(L"renderdoc"))
			myCompilerType = SR_ShaderCompiler_Type::SR_ShaderCompiler_Type_DXBC;
		else
			myCompilerType = SR_ShaderCompiler_Type::SR_ShaderCompiler_Type_DXIL;

		if (myCompilerType == SR_ShaderCompiler_Type_DXIL)
		{
			HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, SR_IID_PPV_ARGS(&myCompiler));
			if (FAILED(hr))
				SC_ERROR_LOG("Failed to create compiler DXC");

			hr = DxcCreateInstance(CLSID_DxcUtils, SR_IID_PPV_ARGS(&myUtils));
			if (FAILED(hr))
				SC_ERROR_LOG("Failed to create instance DXC");

			hr = myUtils->CreateDefaultIncludeHandler(&myIncludeHandler);
			if (FAILED(hr))
				SC_ERROR_LOG("Failed to create default include handler DXC");
		}
	}

	SR_ShaderCompiler_DX12::~SR_ShaderCompiler_DX12()
	{
	}

	SC_Ref<SR_ShaderByteCode> SR_ShaderCompiler_DX12::CompileShaderFromFile(const char* aShaderFile, const SR_ShaderType aShaderType, std::wstring* aDefines, uint aNumDefines)
	{
		std::string generatedHLSLFile;
		std::string hlslCode = LoadHLSL(aShaderFile, generatedHLSLFile);

		SR_ShaderCompileInfo compileInfo;
		compileInfo.myShaderFile = generatedHLSLFile.c_str();
		compileInfo.myDefines.PreAllocate(aNumDefines);
		for (uint i = 0; i < aNumDefines; ++i)
			compileInfo.myDefines.Add(aDefines[i]);

		compileInfo.myEntryPoint = "main";
		compileInfo.myType = aShaderType;

		SC_Ref<SR_ShaderByteCode> shaderByteCode;
		if (myCompilerType == SR_ShaderCompiler_Type_DXBC)
		{
			compileInfo.myTargetProfileDXBC = locGetDXBCTarget(aShaderType);
			compileInfo.myCacheName = locMakeShaderCacheName(hlslCode.c_str(), compileInfo.myTargetProfileDXBC);

			shaderByteCode = FindCompiledByteCode(compileInfo.myCacheName.c_str(), SR_ShaderCompiler_Type_DXBC);
			if (shaderByteCode)
				return shaderByteCode;

			shaderByteCode = CompileInternalDXBC(compileInfo);
		}
		else
		{
			compileInfo.myTargetProfileDXIL = locGetDXILTarget(aShaderType);
			compileInfo.myCacheName = locMakeShaderCacheName(hlslCode.c_str(), compileInfo.myTargetProfileDXIL);

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = hlslCode.data();
			sourceBuffer.Size = hlslCode.size();
			sourceBuffer.Encoding = DXC_CP_UTF8;

			shaderByteCode = FindCompiledByteCode(compileInfo.myCacheName.c_str(), SR_ShaderCompiler_Type_DXIL);
			if (shaderByteCode)
				return shaderByteCode;

			compileInfo.myDxcBuffer = &sourceBuffer;
			shaderByteCode = CompileInternalDXIL(compileInfo);
		}

#if !IS_FINAL_BUILD
		shaderByteCode->myShaderFile = aShaderFile;
#endif

		return shaderByteCode;
	}

	SC_Ref<SR_ShaderByteCode> SR_ShaderCompiler_DX12::CompileShader()
	{
		return nullptr;
	}

	SC_Ref<SR_ShaderByteCode> SR_ShaderCompiler_DX12::CompileInternalDXBC(SR_ShaderCompileInfo& aCompileInfo)
	{
		SC_HybridArray<D3D_SHADER_MACRO, 10> defines;
		for (uint i = 0; i < aCompileInfo.myDefines.Count(); ++i)
		{
			D3D_SHADER_MACRO& define = defines.Add();
			define.Name = ToString(aCompileInfo.myDefines[i]).c_str();
			define.Definition = "1";
		}

		UINT flags = 0;
		if (myDebugShaders)
			flags |= D3DCOMPILE_DEBUG;

		if (mySkipOptimizations)
			flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		else
		{
			if (myShaderOptimizationLevel == 2)
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2; 
			else if (myShaderOptimizationLevel == 1)
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1; 
			else if (myShaderOptimizationLevel == 0)
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
			else
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		}

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile(ToWString(aCompileInfo.myShaderFile).c_str(), defines.GetBuffer(), nullptr, "main", aCompileInfo.myTargetProfileDXBC, flags, 0, &shaderBlob, &errorBlob);
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Failed to compile shader (%s)", aCompileInfo.myShaderFile);
			if (errorBlob)
			{
				SC_ERROR_LOG("Error: %s", (char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return nullptr;
		}

		SC_LOG("Compiled DXBC Shader [%i]", locCompiledShadersCount++);

		ID3D12ShaderReflection* shaderReflection = nullptr;
		hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), SR_IID_PPV_ARGS(&shaderReflection));
		if (FAILED(hr))
			SC_ERROR_LOG("Failed to reflect shader (%s)", aCompileInfo.myShaderFile);

		SC_Ref<SR_ShaderByteCode> bc = new SR_ShaderByteCode();

		bc->myByteCodeSize = static_cast<uint>(shaderBlob->GetBufferSize());
		bc->myByteCode = new uint8[bc->myByteCodeSize];
		SC_Memcpy(bc->myByteCode, shaderBlob->GetBufferPointer(), bc->myByteCodeSize);

		bc->myReflectionData = shaderReflection;

		aCompileInfo.myDXBCBytecode = shaderBlob;
		WriteByteCode(aCompileInfo);

		shaderBlob->Release();

		return bc;
	}

	SC_Ref<SR_ShaderByteCode> SR_ShaderCompiler_DX12::CompileInternalDXIL(SR_ShaderCompileInfo& aCompileInfo)
	{
		SC_GrowingArray<LPCWSTR> compileArgs;
		compileArgs.PreAllocate(13);

		// Entry Point
		compileArgs.Add(L"-E");
		compileArgs.Add(L"main");

		// Target
		compileArgs.Add(L"-T");
		compileArgs.Add(aCompileInfo.myTargetProfileDXIL);

		// Debug Info
		if (myDebugShaders)
		{
			compileArgs.Add(L"-Zi");
			compileArgs.Add(L"-Zss");
			compileArgs.Add(L"-Qstrip_debug");
		}
		if (mySkipOptimizations)
			compileArgs.Add(L"-Od");
		else
		{
			if (myShaderOptimizationLevel == 2)
				compileArgs.Add(L"-O2");
			else if (myShaderOptimizationLevel == 1)
				compileArgs.Add(L"-O1");
			else if (myShaderOptimizationLevel == 0)
				compileArgs.Add(L"-O0");
			else
				compileArgs.Add(L"-O3");
		}

		for (uint i = 0; i < aCompileInfo.myDefines.Count(); ++i)
		{
			compileArgs.Add(L"-D");
			compileArgs.Add(aCompileInfo.myDefines[i].c_str());
		}

		compileArgs.Add(L"-Qstrip_reflect"); 
		compileArgs.Add(L"-flegacy-macro-expansion");

		IDxcResult* compileResults;
		HRESULT hr = myCompiler->Compile(
			aCompileInfo.myDxcBuffer,		// Source buffer.
			compileArgs.GetBuffer(),		// Array of pointers to arguments.
			compileArgs.Count(),			// Number of arguments.
			nullptr,						// User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(&compileResults)	// Compiler output status, buffer, and errors.
		);

		SC_Ref<SR_ShaderByteCode> outShaderByteCode;
		compileResults->GetStatus(&hr);
		if (!FAILED(hr))
		{
			IDxcBlob* shaderByteCode = nullptr;
			hr = compileResults->GetOutput(DXC_OUT_OBJECT, SR_IID_PPV_ARGS(&shaderByteCode), nullptr);

			outShaderByteCode = new SR_ShaderByteCode();
			outShaderByteCode->myByteCodeSize = static_cast<uint>(shaderByteCode->GetBufferSize());
			outShaderByteCode->myByteCode = new uint8[outShaderByteCode->myByteCodeSize];
			SC_Memcpy(outShaderByteCode->myByteCode, shaderByteCode->GetBufferPointer(), outShaderByteCode->myByteCodeSize);

			SC_LOG("Compiled DXIL Shader [%i]", locCompiledShadersCount++);

			IDxcBlob* shaderHashBlob = nullptr;
			hr = compileResults->GetOutput(DXC_OUT_SHADER_HASH, SR_IID_PPV_ARGS(&shaderHashBlob), nullptr);
			std::string shaderName = RetrieveHashFromBlob(shaderHashBlob);

			IDxcBlob* pdbData = nullptr;
			if (myDebugShaders)
			{
				hr = compileResults->GetOutput(DXC_OUT_PDB, SR_IID_PPV_ARGS(&pdbData), nullptr);
				aCompileInfo.myDXILDebugData = pdbData;
			}

			IDxcBlob* reflectionData;
			compileResults->GetOutput(DXC_OUT_REFLECTION, SR_IID_PPV_ARGS(&reflectionData), nullptr);
			DxcBuffer reflectionBuffer;
			reflectionBuffer.Ptr = reflectionData->GetBufferPointer();
			reflectionBuffer.Size = reflectionData->GetBufferSize();
			reflectionBuffer.Encoding = 0;

			ID3D12ShaderReflection* shaderReflection;
			myUtils->CreateReflection(&reflectionBuffer, SR_IID_PPV_ARGS(&shaderReflection));
			outShaderByteCode->myReflectionData = (void*)shaderReflection;

			aCompileInfo.myDXILBytecode = shaderByteCode;
			aCompileInfo.myDXILReflection = reflectionData;

			WriteByteCode(aCompileInfo);
			shaderByteCode->Release();
			reflectionData->Release();
			if (pdbData)
				pdbData->Release();
		}
		else
		{
			SC_ERROR_LOG("Failed to compile shader [%s]", aCompileInfo.myShaderFile);
			IDxcBlobUtf16* name = nullptr;
			IDxcBlobUtf8* errors = nullptr;
			compileResults->GetOutput(DXC_OUT_ERRORS, SR_IID_PPV_ARGS(&errors), &name);
			if (errors != nullptr && errors->GetStringLength() != 0)
				SC_ERROR_LOG("Shader compilation error: %s", errors->GetStringPointer());

			errors->Release();
			if (name)
				name->Release();
		}
		compileResults->Release();
		return outShaderByteCode;
	}

	void SR_ShaderCompiler_DX12::InsertHeaderHLSL(std::string& aShaderCode) const
	{
		if (myDebugShaders)
			aShaderCode.append("// DEBUG-MODE ENABLED\n");

		static constexpr const char* header =
			"#pragma warning(disable: 3206)\n" // Implicit truncation of vector type
			"#pragma warning(disable: 3556)\n" // Interger divides may be slower
			"#pragma warning(disable: 1519)\n" // Macro redefinition

			"float2 SR_Transform(float2x2 aMat, float2 aVec) { return mul(aMat, aVec); }\n"
			"float3 SR_Transform(float3x3 aMat, float3 aVec) { return mul(aMat, aVec); }\n"
			"float4 SR_Transform(float4x4 aMat, float4 aVec) { return mul(aMat, aVec); }\n"
			"float4 SR_Transform(float4x4 aMat, float3 aVec) { return mul(aMat, float4(aVec, 1.0)); }\n"

			"float3x3 SR_ColumnMatrix(float3 x, float3 y, float3 z) { return float3x3(x.x, y.x, z.x, x.y, y.y, z.y, x.z, y.z, z.z); }\n"
			"float3x4 SR_ColumnMatrix(float3 x, float3 y, float3 z, float3 w) { return float3x4(x.x, y.x, z.x, w.x, x.y, y.y, z.y, w.y, x.z, y.z, z.z, w.z); }\n"
			"float4x4 SR_ColumnMatrix(float4 x, float4 y, float4 z, float4 w) { return float4x4(x.x, y.x, z.x, w.x, x.y, y.y, z.y, w.y, x.z, y.z, z.z, w.z, x.w, y.w, z.w, w.w); }\n"
			"float3x3 SR_RowMatrix(float3 x, float3 y, float3 z) { return float3x3(x, y, z); }\n"
			"float3x4 SR_RowMatrix(float4 x, float4 y, float4 z) { return float3x4(x, y, z); }\n"

			"uint2 SR_GetResolutionImpl(RWTexture2D<uint> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(RWTexture2D<float> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(RWTexture2D<float2> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(RWTexture2D<float3> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(RWTexture2D<float4> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(Texture2D<uint> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(Texture2D<float> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(Texture2D<float2> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(Texture2D<float3> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"uint2 SR_GetResolutionImpl(Texture2D<float4> aTexture) { uint2 size; aTexture.GetDimensions(size.x, size.y); return size; }\n"
			"#define SR_GetResolution(texture) SR_GetResolutionImpl(texture)\n"

			"#define SR_Sample(sampler, texture, uv) texture.Sample(sampler, uv)\n"
			"#define SR_SampleLod0(sampler, texture, uv) texture.SampleLevel(sampler, uv, 0)\n"
			"#define SR_SampleLod(sampler, texture, uv, lod) texture.SampleLevel(sampler, uv, lod)\n"
			"#define SR_SampleCmp(sampler, texture, uv, comperand) texture.SampleCmpLevel(sampler, uv, comperand)\n"
			"#define SR_SampleCmpLod0(sampler, texture, uv, comperand) texture.SampleCmpLevelZero(sampler, uv, comperand)\n"
			"#define SR_SampleTexel(texture, location) texture.Load(location)\n"
			"#define SR_SampleTexelLod0(texture, location) texture.Load(int3(location, 0))\n"

			"#define SR_Load(res, location) res.Load(location)\n"
			"#define SR_Load2(res, location) res.Load2(location)\n"
			"#define SR_Load3(res, location) res.Load3(location)\n"
			"#define SR_Load4(res, location) res.Load4(location)\n"

			"#define SR_AtomicMax(dst, value, old) InterlockedMax(dst, value, old)\n"
			"#define SR_AtomicMin(dst, value, old) InterlockedMin(dst, value, old)\n"
			"#define SR_AtomicAdd(dst, value, old) InterlockedAdd(dst, value, old)\n"
			"#define SR_AtomicAnd(dst, value, old) InterlockedAnd(dst, value, old)\n"
			"#define SR_AtomicExchange(dst, value, old) InterlockedExchange(dst, value, old)\n"
			"#define SR_AtomicCompareExchange(dst, comparand, value, old) InterlockedCompareExchange(dst, comparand, value, old)\n"
			"#define SR_AtomicOr(dst, value, old) InterlockedOr(dst, value, old)\n"
			"#define SR_AtomicXor(dst, value, old) InterlockedXor(dst, value, old)\n"

			"#define SR_AllMemoryBarrier AllMemoryBarrier\n"
			"#define SR_AllMemoryBarrierSync AllMemoryBarrierWithGroupSync\n"
			"#define SR_SharedMemoryBarrier DeviceMemoryBarrier\n"
			"#define SR_SharedMemoryBarrierSync DeviceMemoryBarrierWithGroupSync\n"
			"#define SR_GroupMemoryBarrier GroupMemoryBarrier\n"
			"#define SR_GroupMemoryBarrierSync GroupMemoryBarrierWithGroupSync\n"

			"#define SR_Unroll [unroll]\n"
			"#define SR_Loop [loop]\n"
			"#define SR_Branch [branch]\n"
			"#define SR_Flatten [flatten]\n"

			"static const float SR_Pi = 3.14159265358979323846264338328;\n"
			;

		aShaderCode.insert(0, header);
	}

	std::string SR_ShaderCompiler_DX12::LoadHLSL(const char* aShaderFile, std::string& aOutHLSLFile) const
	{
		std::ifstream shaderFile(aShaderFile);

		SC_GrowingArray<std::string> insertedSemantics;
		SC_GrowingArray<std::string> includedFiles;
		std::string shaderCode;
		shaderCode.reserve(4096);

		if (shaderFile.is_open())
		{
			std::string part;
			while (std::getline(shaderFile, part))
			{
				if (part.rfind("#include", 0) == 0 || part.rfind("import", 0) == 0)
					HandleIncludeFile(part, shaderCode, includedFiles, insertedSemantics);
				else
				{
					if (!HandleSemantics(shaderFile, part, shaderCode, insertedSemantics))
						continue;

					shaderCode += part;
					shaderCode += "\n";
				}
			}
		}

		InsertHeaderHLSL(shaderCode);
		shaderFile.close();

		std::string outFilePath = "Assets/Generated/Shaders/HLSL/";
		outFilePath += GetFilename(aShaderFile);
		outFilePath += ".hlsl";
		std::ofstream outf(outFilePath);

		if (outf.is_open())
		{
			aOutHLSLFile = outFilePath;
			outf << shaderCode;
			outf.close();
		}

		return shaderCode;
	}

	void SR_ShaderCompiler_DX12::HandleIncludeFile(const std::string& aIncludeLine, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aIncludedFiles, SC_GrowingArray<std::string>& aInsertedSemantics) const
	{
		std::string includeFile = SC_EnginePaths::GetRelativeDataPath() + std::string("ShiftEngine/Shaders/");

		static constexpr const char delimiter = '"';
		size_t delimiterStart = aIncludeLine.find_first_of(delimiter) + 1;
		size_t delimiterEnd = aIncludeLine.find_last_of(delimiter);
		includeFile += aIncludeLine.substr(delimiterStart, delimiterEnd - delimiterStart);

		if (aIncludedFiles.Find(includeFile) != aIncludedFiles.FoundNone)
			return;

		std::ifstream shaderFile(includeFile);
		if (shaderFile.is_open())
		{
			std::string part;
			while (std::getline(shaderFile, part))
			{
				if (part.rfind("#include", 0) == 0 || part.rfind("import", 0) == 0)
					HandleIncludeFile(part, aOutShaderCode, aIncludedFiles, aInsertedSemantics);
				else
				{
					if (!HandleSemantics(shaderFile, part, aOutShaderCode, aInsertedSemantics))
						continue;

					aOutShaderCode += part;
					aOutShaderCode += "\n";
				}
			}
		}
		aIncludedFiles.Add(includeFile);
		shaderFile.close();
	}

	bool SR_ShaderCompiler_DX12::HandleSemantics(std::ifstream& aShaderFile, const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const
	{
		bool addLineOfCodeToShader = true;
		if (HandleTextureSemantics(aLineOfCode, aOutShaderCode, aInsertedSemantics))
			return false;

		if (HandleBufferSemantics(aLineOfCode, aOutShaderCode, aInsertedSemantics))
			return false;

		if (HandleSamplerSemantics(aLineOfCode, aOutShaderCode, aInsertedSemantics))
			return false;

		if (aLineOfCode.find("SR_ConstantBuffer ") != aLineOfCode.npos)
		{
			addLineOfCodeToShader = false;
			std::string variableName;
			variableName.reserve(64);

			uint registerSlot = SC_UINT_MAX;
			bool isLocal = false;
			if (aLineOfCode.rfind("SR_ConstantBufferRef") != aLineOfCode.npos)
			{
				registerSlot = locGetRefIndex<SR_ConstantBufferRef>(aLineOfCode);
			}
			else if (aLineOfCode.rfind("SR_ConstantBuffer") != aLineOfCode.npos)
			{
				registerSlot = locGetLocalRefIndex<SR_ConstantBufferRef>(aLineOfCode, "SR_ConstantBuffer");
				isLocal = true;
			}

			size_t firstNameCharIdx = aLineOfCode.find(" ") + 1;
			size_t lastNameCharIdx = aLineOfCode.rfind(':') - 1;
			variableName = aLineOfCode.substr(firstNameCharIdx, lastNameCharIdx - firstNameCharIdx);

			std::string structVariables;
			structVariables.reserve(4096);
			structVariables += "cbuffer ";
			structVariables += variableName;
			structVariables += " : ";
			structVariables += "register(b";
			structVariables += std::to_string(registerSlot);
			structVariables += ")\n";
			structVariables += "{\n";

			if (!isLocal)
			{
				structVariables += "\tstruct\n";
				structVariables += "\t{\n";
			}

			std::string nextLine;
			while (std::getline(aShaderFile, nextLine))
			{
				if (nextLine.find('{') != nextLine.npos)
					continue;
				if (nextLine.find('}') != nextLine.npos)
					break;

				if (!isLocal)
					structVariables += '\t';

				structVariables += nextLine;
				structVariables += '\n';
			}

			if (!isLocal)
			{
				structVariables += "\t} ";
				structVariables += variableName;
				structVariables += ";\n";
			}

			structVariables += "}\n";

			if (aInsertedSemantics.AddUnique(structVariables))
				aOutShaderCode += structVariables;
		}
		return addLineOfCodeToShader;
	}

	bool SR_ShaderCompiler_DX12::HandleTextureSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const
	{
		uint registerSlot = SC_UINT_MAX;
		bool isLocal = false;
		bool isRW = false;
		if (aLineOfCode.find("SR_Texture2DRW") != aLineOfCode.npos ||
			aLineOfCode.find("SR_Texture3DRW") != aLineOfCode.npos ||
			aLineOfCode.find("SR_Texture2DArrayRW") != aLineOfCode.npos)
		{
			if (aLineOfCode.rfind("SR_TextureRWRef") != aLineOfCode.npos)
				registerSlot = locGetRefIndex<SR_TextureRWRef>(aLineOfCode);
			else if (aLineOfCode.rfind("SR_TextureRW") != aLineOfCode.npos)
			{
				isLocal = true;
				registerSlot = locGetLocalRefIndex<SR_TextureRWRef>(aLineOfCode, "SR_TextureRW");
			}

			isRW = true;
		}
		else if (aLineOfCode.find("SR_Texture2D") != aLineOfCode.npos ||
			aLineOfCode.find("SR_Texture3D") != aLineOfCode.npos ||
			aLineOfCode.find("SR_TextureCube") != aLineOfCode.npos ||
			aLineOfCode.find("SR_Texture2DArray") != aLineOfCode.npos ||
			aLineOfCode.find("SR_TextureCubeArray") != aLineOfCode.npos)
		{

			if (aLineOfCode.rfind("SR_TextureRef") != aLineOfCode.npos)
				registerSlot = locGetRefIndex<SR_TextureRef>(aLineOfCode);
			else if (aLineOfCode.rfind("SR_Texture") != aLineOfCode.npos)
			{
				isLocal = true;
				registerSlot = locGetLocalRefIndex<SR_TextureRef>(aLineOfCode, "SR_Texture");
			}
		}
		else 
			return false;

		size_t firstNameCharIdx = aLineOfCode.find(" ") + 1;
		size_t lastNameCharIdx = aLineOfCode.rfind(':') - 1;
		std::string variableName = aLineOfCode.substr(firstNameCharIdx, lastNameCharIdx - firstNameCharIdx);
		std::string textureType = locGetTextureType(aLineOfCode);
		std::string dataType = locGetDataType(aLineOfCode);
		std::string output;
		output.reserve(512);
		if (isRW)
			output += "RW";
		output += textureType;
		if (isRW)
		{
			output += "<";
			output += dataType;
			output += "> ";
		}
		output += ' ';
		output += variableName;
		output += " : register(";
		output += (isRW) ? 'u' : 't';
		output += std::to_string(registerSlot);
		output += ", space0);\n";

		if (aInsertedSemantics.AddUnique(output))
		{
			aOutShaderCode += output;
			return true;
		}
		return false;
	}

	bool SR_ShaderCompiler_DX12::HandleBufferSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const
	{
		uint registerSlot = SC_UINT_MAX;
		bool isLocal = false;
		bool isRW = false;
		bool needsDataType = true;
		if (aLineOfCode.find("SR_BufferRW") != aLineOfCode.npos ||
			aLineOfCode.find("SR_ByteBufferRW") != aLineOfCode.npos ||
			aLineOfCode.find("SR_StructuredBufferRW") != aLineOfCode.npos)
		{
			if (aLineOfCode.rfind("SR_BufferRWRef") != aLineOfCode.npos)
				registerSlot = locGetRefIndex<SR_BufferRWRef>(aLineOfCode);
			else if (aLineOfCode.rfind("SR_BufferRW") != aLineOfCode.npos)
			{
				isLocal = true;
				registerSlot = locGetLocalRefIndex<SR_BufferRWRef>(aLineOfCode, "SR_BufferRW");
			}
			isRW = true;
		}
		else if (aLineOfCode.find("SR_Buffer") != aLineOfCode.npos ||
				 aLineOfCode.find("SR_ByteBuffer") != aLineOfCode.npos ||
				 aLineOfCode.find("SR_StructuredBuffer") != aLineOfCode.npos ||
				 aLineOfCode.find("SR_AccelerationStructure") != aLineOfCode.npos)
		{
			if (aLineOfCode.rfind("SR_BufferRef") != aLineOfCode.npos)
				registerSlot = locGetRefIndex<SR_BufferRef>(aLineOfCode);
			else if (aLineOfCode.rfind("SR_Buffer") != aLineOfCode.npos)
			{
				isLocal = true;
				registerSlot = locGetLocalRefIndex<SR_BufferRef>(aLineOfCode, "SR_Buffer");
			}
			
			if (aLineOfCode.find("SR_AccelerationStructure") != aLineOfCode.npos)
				needsDataType = false;
		}
		else
			return false;


		size_t firstNameCharIdx = aLineOfCode.find(" ") + 1;
		size_t lastNameCharIdx = aLineOfCode.rfind(':') - 1;
		std::string variableName = aLineOfCode.substr(firstNameCharIdx, lastNameCharIdx - firstNameCharIdx);
		std::string bufferType = locGetBufferType(aLineOfCode);
		std::string dataType = locGetDataType(aLineOfCode);
		std::string output;
		output.reserve(512);
		if (isRW)
			output += "RW";
		output += bufferType; 
		if (needsDataType)
		{
			output += "<";
			output += dataType;
			output += "> ";
		}
		else
			output += " ";

		output += variableName;
		output += " : register(";
		output += (isRW) ? 'u': 't';
		output += std::to_string(registerSlot);
		output += ", space1);\n";

		if (aInsertedSemantics.AddUnique(output))
		{
			aOutShaderCode += output;
			return true;
		}
		return false;
	}

	bool SR_ShaderCompiler_DX12::HandleSamplerSemantics(const std::string& aLineOfCode, std::string& aOutShaderCode, SC_GrowingArray<std::string>& aInsertedSemantics) const
	{
		uint registerSlot = SC_UINT_MAX;
		bool isLocal = false;
		if (aLineOfCode.find("SR_Sampler") != aLineOfCode.npos)
		{
			if (aLineOfCode.rfind("SR_SamplerRef") != aLineOfCode.npos)
				registerSlot = locGetRefIndex<SR_SamplerRef>(aLineOfCode);
			else if (aLineOfCode.rfind("SR_Sampler") != aLineOfCode.npos)
			{
				isLocal = true;
				registerSlot = locGetLocalRefIndex<SR_SamplerRef>(aLineOfCode, "SR_Sampler");
			}

			size_t firstNameCharIdx = aLineOfCode.find(" ") + 1;
			size_t lastNameCharIdx = aLineOfCode.rfind(':') - 1;
			std::string variableName = aLineOfCode.substr(firstNameCharIdx, lastNameCharIdx - firstNameCharIdx);
			std::string output;
			output.reserve(256);
			output = locGetSamplerType(registerSlot);
			output += " ";
			output += variableName;
			output += " : register(s";
			output += std::to_string(registerSlot);
			output += ");\n";

			if (aInsertedSemantics.AddUnique(output))
			{
				aOutShaderCode += output;
			}
			return true;
		}

		return false;
	}

	void SR_ShaderCompiler_DX12::WriteByteCode(const SR_ShaderCompileInfo& aCompileInfo)
	{
		std::stringstream outFilePath;
		std::filesystem::create_directory("Assets/Generated/Shaders/HLSL/ByteCode/");
		outFilePath << "Assets/Generated/Shaders/HLSL/ByteCode/" << aCompileInfo.myCacheName;

		std::string outputShaderByteCodeFile(outFilePath.str() + std::string(".sshadercache"));
		std::ofstream output(outputShaderByteCodeFile, std::ios::binary);

		if (aCompileInfo.myDXILBytecode)
		{
			uint bytecodeSize = (uint)aCompileInfo.myDXILBytecode->GetBufferSize();
			uint reflectionDataSize = (uint)aCompileInfo.myDXILReflection->GetBufferSize();
			if (output.is_open())
			{
				output.write((char*)&bytecodeSize, sizeof(uint));
				output.write((char*)&reflectionDataSize, sizeof(uint));
				output.write((char*)aCompileInfo.myDXILBytecode->GetBufferPointer(), aCompileInfo.myDXILBytecode->GetBufferSize());

				if (reflectionDataSize)
					output.write((char*)aCompileInfo.myDXILReflection->GetBufferPointer(), aCompileInfo.myDXILReflection->GetBufferSize());
			}

			if (aCompileInfo.myDXILDebugData)
			{
				std::string outputShadePDBFile(outFilePath.str() + std::string(".pdb"));
				std::ofstream outputPDB(outputShadePDBFile, std::ios::binary);
				if (outputPDB.is_open())
				{
					outputPDB.write((char*)aCompileInfo.myDXILDebugData->GetBufferPointer(), aCompileInfo.myDXILDebugData->GetBufferSize());
					outputPDB.close();
				}
			}
		}

		if (aCompileInfo.myDXBCBytecode)
		{
			uint bytecodeSize = (uint)aCompileInfo.myDXBCBytecode->GetBufferSize();
			uint reflectionDataSize = 0;
			if (output.is_open())
			{
				output.write((char*)&bytecodeSize, sizeof(uint));
				output.write((char*)&reflectionDataSize, sizeof(uint));
				output.write((char*)aCompileInfo.myDXBCBytecode->GetBufferPointer(), aCompileInfo.myDXBCBytecode->GetBufferSize());
			}
		}

		output.close();
	}

	SC_Ref<SR_ShaderByteCode> SR_ShaderCompiler_DX12::FindCompiledByteCode(const char* aCacheName, const SR_ShaderCompiler_Type& aCompilerType)
	{
		std::stringstream inFilePath;
		inFilePath << "Assets/Generated/Shaders/HLSL/ByteCode/";
		inFilePath << aCacheName << ".sshadercache";

		std::ifstream input(inFilePath.str(), std::ios::binary);
		if (input.is_open())
		{
			uint bytecodeSize = 0;
			uint reflectionDataSize = 0;
			input.read((char*)&bytecodeSize, sizeof(uint));
			input.read((char*)&reflectionDataSize, sizeof(uint));
			char* bytecode = new char[bytecodeSize];
			input.read(bytecode, bytecodeSize);
			char* reflectionData = new char[reflectionDataSize];
			input.read(reflectionData, reflectionDataSize);
			input.close();

			SC_Ref<SR_ShaderByteCode> shaderByteCode = new SR_ShaderByteCode();
			shaderByteCode->myByteCode = (uint8*)bytecode;
			shaderByteCode->myByteCodeSize = bytecodeSize;

			ID3D12ShaderReflection* shaderReflection = nullptr;
			if (aCompilerType == SR_ShaderCompiler_Type_DXBC)
			{
				HRESULT hr = D3DReflect(bytecode, bytecodeSize, SR_IID_PPV_ARGS(&shaderReflection));
				if (FAILED(hr))
					SC_ERROR_LOG("Failed to reflect shader (%s)", aCacheName);
			}
			else
			{
				DxcBuffer reflectionBuffer;
				reflectionBuffer.Ptr = reflectionData;
				reflectionBuffer.Size = reflectionDataSize;
				reflectionBuffer.Encoding = 0;

				myUtils->CreateReflection(&reflectionBuffer, SR_IID_PPV_ARGS(&shaderReflection));
			}
			shaderByteCode->myReflectionData = (void*)shaderReflection;

			return shaderByteCode;
		}

		return nullptr;
	}
}

#endif