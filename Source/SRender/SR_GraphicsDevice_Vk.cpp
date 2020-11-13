#include "SRender_Precompiled.h"
#include "SR_GraphicsDevice_Vk.h"

#if ENABLE_VULKAN
#include "SR_SwapChain_Vk.h"
#include "SR_Buffer_Vk.h"
#include "SR_BufferView_Vk.h"
#include "SR_TextureBuffer_Vk.h"
#include "SR_Texture_Vk.h"
#include "SR_RenderTarget_Vk.h"

#if IS_WINDOWS
#include "SC_Window_Win64.h"
#elif IS_LINUX
#include "SC_Window_Xlib.h"
#endif


namespace Shift
{
	SR_GraphicsDevice_Vk* SR_GraphicsDevice_Vk::ourGraphicsDevice_Vk = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL locDebugPrintCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* /*pUserData*/) 
	{

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			SC_WARNING_LOG("Vk - Warning: %s", pCallbackData->pMessage);
		}
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			SC_ERROR_LOG("Vk - Error: %s", pCallbackData->pMessage);

		}


		return VK_FALSE;
	}

	VkResult locCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void locDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	SR_GraphicsDevice_Vk::SR_GraphicsDevice_Vk()
		: myPresentQueueIndex(SC_UINT_MAX)
	{
		myAPIType = SR_GraphicsAPI::Vulkan;
		ourGraphicsDevice_Vk = this;

		SC_Fill(mySupportedQueueTypes, 3, false);
	}

	SR_GraphicsDevice_Vk::~SR_GraphicsDevice_Vk()
	{
		vkDestroySurfaceKHR(myVkInstance, myVkSurface, nullptr);
		locDestroyDebugUtilsMessengerEXT(myVkInstance, myDebugMessenger, nullptr);
		vkDestroyDevice(myVkDevice, nullptr);
		vkDestroyInstance(myVkInstance, nullptr);
		ourGraphicsDevice_Vk = nullptr;
	}

	const bool SR_GraphicsDevice_Vk::Init(SC_Window* aWindow)
	{
		myWindow = aWindow;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = aWindow->GetAppName();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Shift Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Enumerate Available Extensions
		SC_GrowingArray<VkExtensionProperties> extensions;
		GetAvailableInstanceExtensions(extensions);

		SC_LOG("Available instance extensions:");
		for (const VkExtensionProperties& prop : extensions)
			SC_LOG("[ %s ] - version %u", prop.extensionName, prop.specVersion);

		SC_HybridArray<const char*, 16> extensionNames;
		if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, extensions))
			extensionNames.Add(VK_KHR_SURFACE_EXTENSION_NAME);

#if IS_WINDOWS
		if (IsExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extensions))
			extensionNames.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif IS_LINUX
		if (IsExtensionSupported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, extensions))
			extensionNames.Add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
#error Platform not supported!
#endif

		if (myEnableDebugging)
			extensionNames.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		createInfo.enabledExtensionCount = extensionNames.Count();
		createInfo.ppEnabledExtensionNames = (createInfo.enabledExtensionCount) ? extensionNames.GetBuffer() : nullptr;

		SC_HybridArray<const char*, 16> layerNames;
		if (myEnableDebugging)
		{
			SC_GrowingArray<VkLayerProperties> validationLayers;
			GetAvailableLayers(validationLayers);

			constexpr const char* debugLayers[] = 
			{
				"VK_LAYER_KHRONOS_validation"
			};

			for (uint i = 0; i < SC_ARRAY_SIZE(debugLayers); ++i)
			{
				if (IsLayerSupported(debugLayers[i], validationLayers))
					layerNames.Add(debugLayers[i]);
			}
		}
		createInfo.enabledLayerCount = layerNames.Count();
		createInfo.ppEnabledLayerNames = (createInfo.enabledLayerCount) ? layerNames.GetBuffer() : nullptr; 

		if (vkCreateInstance(&createInfo, nullptr, &myVkInstance) != VK_SUCCESS) 
		{
			SC_ERROR_LOG("Failed to create VkInstance!");
			return false;
		}

		if (myEnableDebugging)
		{
			VkDebugUtilsMessengerCreateInfoEXT dbgMessagerCreateInfo{};
			dbgMessagerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			dbgMessagerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			dbgMessagerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			dbgMessagerCreateInfo.pfnUserCallback = locDebugPrintCallback;
			dbgMessagerCreateInfo.pUserData = nullptr; // Optional

			if (locCreateDebugUtilsMessengerEXT(myVkInstance, &dbgMessagerCreateInfo, nullptr, &myDebugMessenger) != VK_SUCCESS)
			{
				SC_ERROR_LOG("Failed to set up debug messenger!");
				return false;
			}
		}

		if (!SetupSurface())
			return false;

		if (!SetupDevice(layerNames))
			return false;

		if (!PostInit())
			return false;

		return true;
	}

	void SR_GraphicsDevice_Vk::BeginFrame()
	{

	}

	void SR_GraphicsDevice_Vk::EndFrame()
	{

	}

	void SR_GraphicsDevice_Vk::Present()
	{

	}

	SR_GraphicsContext* SR_GraphicsDevice_Vk::GetContext(const SR_ContextType& /*aType*/)
	{

		return nullptr;
	}

#if SR_ENABLE_RAYTRACING
	void SR_GraphicsDevice_Vk::SetRaytracingInstanceData(SR_RaytracingInstanceData& /*aOutInstance*/, SR_Buffer* /*aMeshBuffer*/, const SC_Matrix44& /*aMatrix*/, uint /*aInstanceData*/, uint8 /*aMask*/, uint8 /*aHitGroup*/, bool /*aIsOpaque*/, SR_RasterizerFaceCull /*aCullMode*/)
	{

	}

#endif

	SC_Ref<SR_TextureBuffer> SR_GraphicsDevice_Vk::CreateTextureBuffer(const SR_TextureBufferDesc& aDesc, const char* aID /*= nullptr*/)
	{
		bool isRenderTarget = (aDesc.myFlags & SR_ResourceFlag_AllowRenderTarget);
		bool isDepth = (aDesc.myFlags & SR_ResourceFlag_AllowDepthStencil);

		SC_Ref<SR_TextureBuffer_Vk> textureBuffer(new SR_TextureBuffer_Vk());
		textureBuffer->myProperties = aDesc;

		VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		const uint arraySizeScale = aDesc.myDimension == SR_Dimension_TextureCube ? 6 : 1;

		imageCreateInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		imageCreateInfo.imageType = SR_GetImageType_Vk(aDesc.myDimension);
		imageCreateInfo.format = SR_ConvertFormat_Vk(aDesc.myFormat);
		imageCreateInfo.mipLevels = aDesc.myMips;
		imageCreateInfo.arrayLayers = arraySizeScale * SC_Max<uint>(aDesc.myArraySize, 1u);
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = VkExtent3D{ uint(aDesc.myWidth), uint(aDesc.myHeight), uint(aDesc.myDepth) };

		VkImageUsageFlags usageFlags = 0;
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

		if (isDepth)
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else if (isRenderTarget)
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (aDesc.myFlags & SR_ResourceFlag_AllowWrites)
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		
		imageCreateInfo.usage = usageFlags;

		if (aDesc.myDimension == SR_Dimension_TextureCube)
		{
			imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			textureBuffer->myProperties.myArraySize = 6;
		}

		if (vkCreateImage(myVkDevice, &imageCreateInfo, nullptr, &textureBuffer->myVkImage) != VK_SUCCESS)
		{
			SC_ERROR_LOG("Could not create texture: %s", aID);
			return nullptr;
		}

		return textureBuffer;
	}

	SC_Ref<SR_Texture> SR_GraphicsDevice_Vk::CreateTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SC_Ref<SR_Texture_Vk> texture = new SR_Texture_Vk();
		SR_TextureBuffer_Vk* buffer = static_cast<SR_TextureBuffer_Vk*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = buffer->GetProperties();
		texture->myTextureBuffer = buffer;

		const VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: Check for other types

		const uint storageArraySize = SC_Max(aTextureDesc.myArraySize, 1u);
		const uint arraySize = aTextureDesc.myArraySize ? aTextureDesc.myArraySize : storageArraySize - aTextureDesc.myFirstArrayIndex;
		SC_ASSERT(arraySize <= storageArraySize);

		const uint mip = aTextureDesc.myMostDetailedMip;
		const uint mipLevels = aTextureDesc.myMipLevels ? aTextureDesc.myMipLevels : buffer->myProperties.myMips - mip;
		SC_ASSERT(mip + mipLevels <= buffer->myProperties.myMips);

		VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = buffer->myVkImage;

		createInfo.format = SR_ConvertFormat_Vk(aTextureDesc.myFormat == SR_Format_Unknown ? bufferProps.myFormat : aTextureDesc.myFormat);
		createInfo.components = 
		{ 
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A 
		};

		createInfo.subresourceRange = {};
		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = mip;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = aTextureDesc.myFirstArrayIndex;
		createInfo.subresourceRange.layerCount = arraySize;

		if (buffer->GetProperties().myDimension == SR_Dimension_TextureCube)
		{
			createInfo.subresourceRange.layerCount *= 6;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
			createInfo.viewType = SR_GetImageViewType_Vk(bufferProps.myDimension, bufferProps.myArraySize > 1);

		if (vkCreateImageView(myVkDevice, &createInfo, nullptr, &texture->myVkImageView) != VK_SUCCESS)
		{
			SC_ERROR_LOG("Could not create texture");
			return nullptr;
		}

		return texture;
	}

	SC_Ref<SR_Texture> SR_GraphicsDevice_Vk::CreateRWTexture(const SR_TextureDesc& aTextureDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SC_Ref<SR_Texture_Vk> texture = new SR_Texture_Vk();
		SR_TextureBuffer_Vk* buffer = static_cast<SR_TextureBuffer_Vk*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = buffer->GetProperties();
		texture->myTextureBuffer = buffer;

		const VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: Check for other types

		const uint storageArraySize = SC_Max(aTextureDesc.myArraySize, 1u);
		const uint arraySize = aTextureDesc.myArraySize ? aTextureDesc.myArraySize : storageArraySize - aTextureDesc.myFirstArrayIndex;
		SC_ASSERT(arraySize <= storageArraySize);

		const uint mip = aTextureDesc.myMostDetailedMip;
		const uint mipLevels = aTextureDesc.myMipLevels ? aTextureDesc.myMipLevels : buffer->myProperties.myMips - mip;
		SC_ASSERT(mip + mipLevels <= buffer->myProperties.myMips);

		VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = buffer->myVkImage;

		createInfo.format = SR_ConvertFormat_Vk(aTextureDesc.myFormat == SR_Format_Unknown ? bufferProps.myFormat : aTextureDesc.myFormat);
		createInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};

		createInfo.subresourceRange = {};
		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = mip;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = aTextureDesc.myFirstArrayIndex;
		createInfo.subresourceRange.layerCount = arraySize;

		if (buffer->GetProperties().myDimension == SR_Dimension_TextureCube)
		{
			createInfo.subresourceRange.layerCount *= 6;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
		else
			createInfo.viewType = SR_GetImageViewType_Vk(bufferProps.myDimension, bufferProps.myArraySize > 1);

		if (vkCreateImageView(myVkDevice, &createInfo, nullptr, &texture->myVkImageView) != VK_SUCCESS)
		{
			SC_ERROR_LOG("Could not create texture");
			return nullptr;
		}

		return texture;
	}

	SC_Ref<SR_Texture> SR_GraphicsDevice_Vk::GetCreateTexture(const char* /*aFile*/)
	{

		return nullptr;
	}

	SC_Ref<SR_RenderTarget> SR_GraphicsDevice_Vk::CreateRenderTarget(const SR_RenderTargetDesc& aRenderTargetDesc, SR_TextureBuffer* aTextureBuffer)
	{
		SC_Ref<SR_RenderTarget_Vk> rt = new SR_RenderTarget_Vk();
		SR_TextureBuffer_Vk* buffer = static_cast<SR_TextureBuffer_Vk*>(aTextureBuffer);
		const SR_TextureBufferDesc& bufferProps = buffer->GetProperties();
		rt->myTextureBuffer = aTextureBuffer;

		VkImageViewCreateInfo vkCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		vkCreateInfo.format = SR_ConvertFormat_Vk(aRenderTargetDesc.myFormat == SR_Format_Unknown ? bufferProps.myFormat : aRenderTargetDesc.myFormat);
		vkCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		vkCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vkCreateInfo.flags = 0;
		vkCreateInfo.image = buffer->myVkImage;
		VkImageSubresourceRange& subResource = vkCreateInfo.subresourceRange;
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseMipLevel = 0;
		subResource.levelCount = 1;
		subResource.baseArrayLayer = aRenderTargetDesc.myArrayIndex;

		if (bufferProps.myDimension == SR_Dimension_TextureCube)
			subResource.baseArrayLayer = (subResource.baseArrayLayer * 6) + aRenderTargetDesc.myArrayIndex;

		subResource.layerCount = 1;

		if (vkCreateImageView(myVkDevice, &vkCreateInfo, nullptr, &rt->myVkImageView) != VK_SUCCESS)
		{
			SC_ERROR_LOG("Could not create render target");
			return nullptr;
		}

		return rt;
	}

	SC_Ref<SR_RenderTarget> SR_GraphicsDevice_Vk::CreateDepthStencil(const SR_RenderTargetDesc& /*aRenderTargetDesc*/, SR_TextureBuffer* /*aTextureBuffer*/)
	{

		return nullptr;
	}

	SC_Ref<SR_Buffer> SR_GraphicsDevice_Vk::CreateBuffer(const SR_BufferDesc& /*aBufferDesc*/, void* /*aInitialData*/ /*= nullptr*/, const char* /*aIdentifier*/ /*= nullptr*/)
	{

		return nullptr;
	}

	SC_Ref<SR_BufferView> SR_GraphicsDevice_Vk::CreateBufferView(const SR_BufferViewDesc& /*aViewDesc*/, SR_Buffer* /*aBuffer*/)
	{

		return nullptr;
	}

	SC_Ref<SR_ShaderState> SR_GraphicsDevice_Vk::CreateShaderState(const SR_ShaderStateDesc& /*aShaderStateDesc*/)
	{

		return nullptr;
	}

	void SR_GraphicsDevice_Vk::GenerateMips(SR_TextureBuffer* /*aTextureBuffer*/)
	{

	}

	const SC_Vector2f& SR_GraphicsDevice_Vk::GetResolution() const
	{
		return myWindow->GetResolution();
	}

	SR_GraphicsDeviceStats SR_GraphicsDevice_Vk::GetStats() const
	{
		return SR_GraphicsDeviceStats();
	}

	SR_GraphicsDevice_Vk* SR_GraphicsDevice_Vk::GetVkDevice()
	{
		return ourGraphicsDevice_Vk;
	}

	const bool SR_GraphicsDevice_Vk::PostInit()
	{
		SR_SwapChainDesc swapChainDesc;
		swapChainDesc.myWindow = myWindow;
		swapChainDesc.myBackbufferFormat = SR_Format_RGBA8_Unorm;
		mySwapChain = new SR_SwapChain_Vk();
		if (!mySwapChain->Init(swapChainDesc))
			return false;

		return true;
	}

	void SR_GraphicsDevice_Vk::SetFeatureSupport()
	{
	}

	void SR_GraphicsDevice_Vk::InactivateContextInternal(SR_GraphicsContext* /*aCtx*/)
	{
	}

	void SR_GraphicsDevice_Vk::Destroy()
	{

	}

	void SR_GraphicsDevice_Vk::GetAvailableLayers(SC_GrowingArray<VkLayerProperties>& aOutAvailableLayers) const
	{
		uint layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		if (layerCount)
		{
			aOutAvailableLayers.Respace(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, aOutAvailableLayers.GetBuffer());
		}
	}

	void SR_GraphicsDevice_Vk::GetAvailableInstanceExtensions(SC_GrowingArray<VkExtensionProperties>& aOutAvailableExtensions) const
	{
		uint extensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		if (extensionCount)
		{
			aOutAvailableExtensions.Respace(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, aOutAvailableExtensions.GetBuffer());
		}
	}

	void SR_GraphicsDevice_Vk::GetAvailableDeviceExtensions(VkPhysicalDevice aDevice, SC_GrowingArray<VkExtensionProperties>& aOutAvailableExtensions) const
	{
		uint extensionCount;
		vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &extensionCount, nullptr);

		if (extensionCount)
		{
			aOutAvailableExtensions.Respace(extensionCount);
			vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &extensionCount, aOutAvailableExtensions.GetBuffer());
		}
	}

	bool SR_GraphicsDevice_Vk::IsLayerSupported(const char* aLayerName, const SC_GrowingArray<VkLayerProperties>& aAvailableLayers) const
	{
		for (const VkLayerProperties& layerProps : aAvailableLayers)
		{
			if (SC_Strcmp(layerProps.layerName, aLayerName))
				return true;
		}

		SC_ERROR_LOG("Validation Layer not supported: (%s)", aLayerName);
		return false;
	}

	SC_PRAGMA_DEOPTIMIZE
	bool SR_GraphicsDevice_Vk::IsExtensionSupported(const char* aExtensionName, const SC_GrowingArray<VkExtensionProperties>& aAvailableExtensions) const
	{
		for (const VkExtensionProperties& extensionProps : aAvailableExtensions)
		{
			if (SC_Strcmp(extensionProps.extensionName, aExtensionName))
			{
				return true;
			}
		}

		uint count = 0;
		if (vkEnumerateInstanceExtensionProperties(aExtensionName, &count, nullptr) == VK_SUCCESS)
			return true;
			
		SC_ERROR_LOG("Extension not supported: (%s)", aExtensionName);
		return false;
	}

	bool SR_GraphicsDevice_Vk::SetupSurface()
	{
#if IS_WINDOWS

		SC_Window_Win64* window = static_cast<SC_Window_Win64*>(myWindow);

		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = window->GetHandle();
		createInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(myVkInstance, &createInfo, nullptr, &myVkSurface) != VK_SUCCESS) 
		{
			SC_ERROR_LOG("Failed to create window surface!");
			return false;
		}

		return true;

#elif IS_LINUX

		SC_Window_Xlib* window = static_cast<SC_Window_Xlib*>(myWindow);

		VkXlibSurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		//createInfo.dpy = window->GetHandle();
		//createInfo.window = GetModuleHandle(nullptr);

		if (vkCreateXlibSurfaceKHR(myVkInstance, &createInfo, nullptr, &myVkSurface) != VK_SUCCESS)
		{
			SC_ERROR_LOG("Failed to create window surface!");
			return false;
		}

		return true;

#else
		return false;
#endif
	}

	bool SR_GraphicsDevice_Vk::SetupDevice(const SC_HybridArray<const char*, 16>& aValidationLayers)
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			SC_ERROR_LOG("No compatible GPU found.");
			return false;
		}

		SC_GrowingArray<VkPhysicalDevice> devices;
		devices.Respace(deviceCount);
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, devices.GetBuffer());

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		uint deviceIndex = 0;
		for (const VkPhysicalDevice& device : devices)
		{
			deviceProperties = {};
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			deviceFeatures = {};
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				break;
			++deviceIndex;
		}

		myVkPhysicalDevice = devices[deviceIndex];

		uint queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(myVkPhysicalDevice, &queueFamilyCount, nullptr);

		SC_GrowingArray<VkQueueFamilyProperties> queueFamilies;
		queueFamilies.Respace(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(myVkPhysicalDevice, &queueFamilyCount, queueFamilies.GetBuffer());

		uint queueFamilyIndices[3] = { SC_UINT_MAX, SC_UINT_MAX, SC_UINT_MAX };
		uint index = 0;
		VkBool32 presentSupport = false;
		for (const VkQueueFamilyProperties& queueProps : queueFamilies)
		{
			if (queueProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				mySupportedQueueTypes[0] = true;
				queueFamilyIndices[0] = index;
			}
			else if (queueProps.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				mySupportedQueueTypes[1] = true;
				queueFamilyIndices[1] = index;
			}
			else if (queueProps.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				mySupportedQueueTypes[2] = true;
				queueFamilyIndices[2] = index;
			}

			if (!presentSupport)
			{
				vkGetPhysicalDeviceSurfaceSupportKHR(myVkPhysicalDevice, index, myVkSurface, &presentSupport);
				if (presentSupport && myPresentQueueIndex == SC_UINT_MAX)
					myPresentQueueIndex = index;
			}
			++index;
		}

		SC_GrowingArray<VkExtensionProperties> availableExtensions;
		GetAvailableDeviceExtensions(myVkPhysicalDevice, availableExtensions);
		SC_LOG("Available device extensions:");
		for (const VkExtensionProperties& prop : availableExtensions)
			SC_LOG("[ %s ] - version %u", prop.extensionName, prop.specVersion);

		float queuePriority = 1.0f;
		SC_HybridArray<VkDeviceQueueCreateInfo, 3> queueCreateInfos;
		for (uint i = 0; i < 3; ++i)
		{
			if (mySupportedQueueTypes[i])
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamilyIndices[i];
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.Add(SC_Move(queueCreateInfo));
			}
		}

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = queueCreateInfos.Count();
		createInfo.pQueueCreateInfos = queueCreateInfos.GetBuffer();
		createInfo.enabledLayerCount = aValidationLayers.Count();
		createInfo.ppEnabledLayerNames = (createInfo.enabledLayerCount) ? aValidationLayers.GetBuffer() : nullptr;
		createInfo.pEnabledFeatures = &deviceFeatures;

		SC_HybridArray<const char*, 16> extensionNames;

		if (IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, availableExtensions))
			extensionNames.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#if SR_ENABLE_MESH_SHADERS
		if (IsExtensionSupported(VK_NV_MESH_SHADER_EXTENSION_NAME, availableExtensions))
		{
			SC_LOG("Mesh Shaders: true");
			extensionNames.Add(VK_NV_MESH_SHADER_EXTENSION_NAME);
			myFeatureSupport.myEnableMeshShaders = true;
		}
		else
			SC_LOG("Mesh Shaders: false");
#endif

#if SR_ENABLE_RAYTRACING
		if (IsExtensionSupported(VK_KHR_RAY_TRACING_EXTENSION_NAME, availableExtensions))
		{
			SC_LOG("Raytracing: true");
			extensionNames.Add(VK_KHR_RAY_TRACING_EXTENSION_NAME);
			myFeatureSupport.myEnableRaytracing = true;
		}
		else
			SC_LOG("Raytracing: false");
#endif

		createInfo.enabledExtensionCount = extensionNames.Count();
		createInfo.ppEnabledExtensionNames = (createInfo.enabledExtensionCount) ? extensionNames.GetBuffer() : nullptr;

		if (vkCreateDevice(myVkPhysicalDevice, &createInfo, nullptr, &myVkDevice) != VK_SUCCESS) 
		{
			SC_ERROR_LOG("Failed to create logical device!");
			return false;
		}

		return true;
	}

}

#endif