#pragma once
#include "SR_GraphicsDevice.h"

#if ENABLE_VULKAN

#include "SR_ForwardDeclarations_Vk.h"
#include "SR_GraphicsResources.h"

struct VkLayerProperties;
struct VkExtensionProperties;

namespace Shift
{
	class SR_SwapChain_Vk;
	class SR_GraphicsDevice_Vk final : public SR_GraphicsDevice
	{
	public:
		SR_GraphicsDevice_Vk();
		~SR_GraphicsDevice_Vk();

		const bool Init(SC_Window* aWindow) override;
		void BeginFrame() override;
		void EndFrame() override;

		void Present() override;

		SR_GraphicsContext* GetContext(const SR_ContextType& aType) override;

#if SR_ENABLE_RAYTRACING
		void SetRaytracingInstanceData(SR_RaytracingInstanceData& aOutInstance, SR_Buffer* aMeshBuffer, const SC_Matrix44& aMatrix, uint aInstanceData, uint8 aMask, uint8 aHitGroup, bool aIsOpaque, SR_RasterizerFaceCull aCullMode) override;
#endif
		//////////////////////
		// Resources

		// Textures
		SC_Ref<SR_TextureBuffer> CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID = nullptr) override;

		SC_Ref<SR_Texture> CreateTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_Texture> CreateRWTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_Texture> GetCreateTexture(const char* aFile) override;

		SC_Ref<SR_RenderTarget> CreateRenderTarget(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) override;
		SC_Ref<SR_RenderTarget> CreateDepthStencil(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer) override;

		// Buffers
		SC_Ref<SR_Buffer> CreateBuffer(const SR_BufferDesc& aBufferDesc, void* aInitialData = nullptr, const char* aIdentifier = nullptr) override;
		SC_Ref<SR_BufferView> CreateBufferView(const SR_BufferViewDesc& aViewDesc, SR_Buffer* aBuffer) override;


		// Shaders
		SC_Ref<SR_ShaderState> CreateShaderState(const SR_ShaderStateDesc& aShaderStateDesc) override;

		void GenerateMips(SR_TextureBuffer* aTextureBuffer) override;

		const SC_Vector2f& GetResolution() const override;
		SR_GraphicsDeviceStats GetStats() const override;

		// Vulkan specific
		static SR_GraphicsDevice_Vk* GetVkDevice();

		SR_SwapChain_Vk* GetSwapChain() const { return mySwapChain; }

		VkInstance GetVkInstance() const { return myVkInstance; }
		VkSurfaceKHR GetVkSurface() const { return myVkSurface; }
		VkDevice GetNativeDevice() const { return myVkDevice; }
		VkPhysicalDevice GetNativePhysicalDevice() const { return myVkPhysicalDevice; }
		const uint GetPresentQueueIndex() const { return myPresentQueueIndex; }

	private:
		const bool PostInit() override;
		void SetFeatureSupport() override;
		void InactivateContextInternal(SR_GraphicsContext* aCtx) override;
		void Destroy();
		void GetAvailableLayers(SC_GrowingArray<VkLayerProperties>& aOutAvailableLayers) const;
		void GetAvailableInstanceExtensions(SC_GrowingArray<VkExtensionProperties>& aOutAvailableExtensions) const;
		void GetAvailableDeviceExtensions(VkPhysicalDevice aDevice, SC_GrowingArray<VkExtensionProperties>& aOutAvailableExtensions) const;
		bool IsLayerSupported(const char* aLayerName, const SC_GrowingArray<VkLayerProperties>& aAvailableLayers) const;
		bool IsExtensionSupported(const char* aExtensionName, const SC_GrowingArray<VkExtensionProperties>& aAvailableExtensions) const;
		bool SetupSurface();
		bool SetupDevice(const SC_HybridArray<const char*, 16>& aValidationLayers);

		static SR_GraphicsDevice_Vk* ourGraphicsDevice_Vk;

		SC_Window* myWindow;

		VkInstance myVkInstance;
		VkSurfaceKHR myVkSurface;
		VkDevice myVkDevice;
		VkPhysicalDevice myVkPhysicalDevice;

		SC_UniquePtr<SR_SwapChain_Vk> mySwapChain;

		VkDebugUtilsMessengerEXT myDebugMessenger;

		bool mySupportedQueueTypes[3];
		uint myPresentQueueIndex;
	};
}
#endif