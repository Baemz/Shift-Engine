#pragma once
#include "SR_RenderDefines.h"

#if ENABLE_DX12

#define ENABLE_PIX !IS_FINAL_BUILD

#include SR_INCLUDE_FILE_D3D12 
#include SR_INCLUDE_FILE_DXGI
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

// For Shader compiler
#include SR_INCLUDE_FILE_D3DCOMPILER
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"D3DCompiler.lib")

#if ENABLE_PIX

#	define USE_PIX
#	include "pix3.h"
#	pragma comment(lib,"WinPixEventRuntime.lib")

inline void PIXSetMarkerOnContext(_In_ ID3D12GraphicsCommandList* commandList, _In_reads_bytes_(size) void* data, UINT size)
{
	commandList->SetMarker(D3D12_EVENT_METADATA, data, size);
}

inline void PIXSetMarkerOnContext(_In_ ID3D12CommandQueue* commandQueue, _In_reads_bytes_(size) void* data, UINT size)
{
	commandQueue->SetMarker(D3D12_EVENT_METADATA, data, size);
}

inline void PIXBeginEventOnContext(_In_ ID3D12GraphicsCommandList* commandList, _In_reads_bytes_(size) void* data, UINT size)
{
	commandList->BeginEvent(D3D12_EVENT_METADATA, data, size);
}

inline void PIXBeginEventOnContext(_In_ ID3D12CommandQueue* commandQueue, _In_reads_bytes_(size) void* data, UINT size)
{
	commandQueue->BeginEvent(D3D12_EVENT_METADATA, data, size);
}
inline void PIXEndEventOnContext(_In_ ID3D12GraphicsCommandList* commandList)
{
	commandList->EndEvent();
}

inline void PIXEndEventOnContext(_In_ ID3D12CommandQueue* commandQueue)
{
	commandQueue->EndEvent();
}
#endif

#endif