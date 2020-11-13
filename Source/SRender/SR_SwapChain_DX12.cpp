#include "SRender_Precompiled.h"
#include "SR_SwapChain_DX12.h"

#if ENABLE_DX12

#include "SR_QueueManager_DX12.h"
#include "SR_Buffer_DX12.h"
#include "SR_TextureBuffer_DX12.h"

#include "SC_Window_Win64.h"

namespace Shift
{
	SR_SwapChain_DX12::SR_SwapChain_DX12()
		: myAllowTearing(false)
	{
	}
	SR_SwapChain_DX12::~SR_SwapChain_DX12()
	{
		if (mySwapchain)
			mySwapchain->Release();
		if (mySwapchain4)
			mySwapchain4->Release();
	}
	bool SR_SwapChain_DX12::Init(const SR_SwapChainDesc& aDesc)
	{
		SC_Window_Win64* window = static_cast<SC_Window_Win64*>(aDesc.myWindow);

		DXGI_MODE_DESC backBufferDesc = {};
		backBufferDesc.Width = (uint)window->GetResolution().x;
		backBufferDesc.Height = (uint)window->GetResolution().y;
		backBufferDesc.Format = SR_ConvertFormat_DX12(aDesc.myBackbufferFormat);

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = ourNumBackbuffers;
		swapChainDesc.BufferDesc = backBufferDesc;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = window->GetHandle();
		swapChainDesc.SampleDesc = sampleDesc;
		swapChainDesc.Windowed = !window->IsFullscreen();

		IDXGIFactory6* dxgiFactory;
		HRESULT hr = CreateDXGIFactory1(SR_IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not create swapchain factory.");
			return false;
		}

		bool allowTearing = false;
		hr = dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		myAllowTearing = SUCCEEDED(hr) && allowTearing;
		swapChainDesc.Flags = myAllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		SR_QueueManager_DX12* queueManager = static_cast<SR_QueueManager_DX12*>(SR_QueueManager::Get());

		hr = dxgiFactory->CreateSwapChain(queueManager->GetQueue(SR_ContextType_Render), &swapChainDesc, &mySwapchain);
		if (FAILED(hr))
		{
			SC_ERROR_LOG("Could not create swapchain.");
			return false;
		}

		hr = mySwapchain->QueryInterface(SR_IID_PPV_ARGS(&mySwapchain4));

		dxgiFactory->Release();

		for (int i = 0; i < ourNumBackbuffers; i++)
		{
			ID3D12Resource* resource;
			hr = mySwapchain->GetBuffer(i, SR_IID_PPV_ARGS(&resource));
			if (FAILED(hr))
			{
				return false;
			}
			std::wstring name(L"Framebuffer Texture " + std::to_wstring(i));
			resource->SetName(name.c_str());

			D3D12_RESOURCE_DESC desc = resource->GetDesc();
			SR_TextureBufferDesc textureBufDesc;

			textureBufDesc.myWidth = static_cast<float>(desc.Width);
			textureBufDesc.myHeight = static_cast<float>(desc.Height);
			textureBufDesc.myMips = desc.MipLevels;
			textureBufDesc.myFormat = SR_ConvertFormatFrom_DX12(desc.Format);
			textureBufDesc.myFlags = desc.Flags;
			textureBufDesc.myDimension = static_cast<SR_Dimension>(desc.Dimension);

			SC_Ref<SR_TextureBuffer_DX12> texBuf = new SR_TextureBuffer_DX12();
			texBuf->myDX12Resource = resource;
			texBuf->myProperties = textureBufDesc;

			SR_TextureDesc textureDesc;
			textureDesc.myFormat = textureBufDesc.myFormat;
			textureDesc.myMipLevels = textureBufDesc.myMips;

			myScreenTextures[i] = SR_GraphicsDevice::GetDevice()->CreateTexture(textureDesc, texBuf);

			SR_RenderTargetDesc rtDesc;
			rtDesc.myFormat = textureBufDesc.myFormat;
			myScreenTargets[i] = SR_GraphicsDevice::GetDevice()->CreateRenderTarget(rtDesc, texBuf);
		}

		if (mySwapchain4)
		{
			SC_LOG("SwapChain version: IDXGISwapChain4");
			SC_Atomic_CompareExchange(myCurrentBackbufferIndex, mySwapchain4->GetCurrentBackBufferIndex(), myCurrentBackbufferIndex);
		}
		else
			SC_LOG("SwapChain version: IDXGISwapChain");

		SC_SUCCESS_LOG("Successfully created IDXGISwapChain");
		return true;
	}
	void SR_SwapChain_DX12::Present(bool aSync)
	{
		uint syncInterval = (aSync ? 1 : 0);
		uint presentFlags = (aSync ? 0 : (myAllowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0));

		HRESULT hr = mySwapchain->Present(syncInterval, presentFlags);
		if (FAILED(hr))
			assert(false && "Could not present.");

		uint nextIndex;
		if (mySwapchain4)
			nextIndex = mySwapchain4->GetCurrentBackBufferIndex();
		else
			nextIndex = (myCurrentBackbufferIndex == ourNumBackbuffers - 1) ? 0 : myCurrentBackbufferIndex + 1;

		SC_Atomic_CompareExchange(myCurrentBackbufferIndex, nextIndex, myCurrentBackbufferIndex);
	}
}
#endif