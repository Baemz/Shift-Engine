#include "SRender_Precompiled.h"
#include "SR_SwapChain_Vk.h"

#if ENABLE_VULKAN
#if IS_WINDOWS
#include "SC_Window_Win64.h"

#elif IS_LINUX
#include "SC_Window_Xlib.h"
#endif

#include "SR_GraphicsDevice_Vk.h"
#include "SR_TextureBuffer_Vk.h"

namespace Shift
{

	VkSurfaceFormatKHR locChooseFormat(const VkFormat& aTargetFormat, const SC_GrowingArray<VkSurfaceFormatKHR>& aFormats)
	{
		for (const VkSurfaceFormatKHR& availableFormat : aFormats)
		{
			if (availableFormat.format == aTargetFormat && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return aFormats[0];
	}

	VkPresentModeKHR locChoosePresentMode(const SC_GrowingArray<VkPresentModeKHR>& aAvailablePresentModes) 
	{
		for (const auto& availablePresentMode : aAvailablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	SR_SwapChain_Vk::SR_SwapChain_Vk()
	{

	}

	SR_SwapChain_Vk::~SR_SwapChain_Vk()
	{
		vkDestroySwapchainKHR(myDevice, mySwapChain, nullptr);
	}

	bool SR_SwapChain_Vk::Init(const SR_SwapChainDesc& aDesc)
	{
		SR_GraphicsDevice_Vk* sr_device = SR_GraphicsDevice_Vk::GetVkDevice();
		VkSurfaceKHR surface = sr_device->GetVkSurface();
		VkPhysicalDevice physicalDevice = sr_device->GetNativePhysicalDevice();
		myDevice = sr_device->GetNativeDevice();

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

		SC_GrowingArray<VkSurfaceFormatKHR> formats;
		uint formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		if (formatCount != 0) 
		{
			formats.Respace(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.GetBuffer());
		}
		VkSurfaceFormatKHR surfaceFormat = locChooseFormat(SR_ConvertFormat_Vk(aDesc.myBackbufferFormat), formats);
		myFormat = SR_ConvertFormatFrom_Vk(surfaceFormat.format);

		SC_GrowingArray<VkPresentModeKHR> presentModes;
		uint presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) 
		{
			presentModes.Respace(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.GetBuffer());
		}
		VkPresentModeKHR presentMode = locChoosePresentMode(presentModes);

		VkExtent2D resolution;
#if IS_WINDOWS
		SC_Window_Win64* window = static_cast<SC_Window_Win64*>(aDesc.myWindow);
		resolution.width = (uint)window->GetResolution().x;
		resolution.height = (uint)window->GetResolution().y;
#elif IS_LINUX
		SC_Window_Xlib* window = static_cast<SC_Window_Xlib*>(aDesc.myWindow);
		resolution.width = (uint)window->GetResolution().x;
		resolution.height = (uint)window->GetResolution().y;
#else
#	error Platform not supported!
#endif
		
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = sr_device->GetVkSurface();
		createInfo.minImageCount = SC_Clamp(ourNumBackbuffers, capabilities.minImageCount, capabilities.maxImageCount);
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = resolution;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; 
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &mySwapChain) != VK_SUCCESS) 
		{
			SC_ERROR_LOG("Failed to create SwapChain!");
			return false;
		}

		GetBackbufferTextures();
		return true;
	}
	 
	void SR_SwapChain_Vk::Present(bool /*aSync*/)
	{

	}

	void SR_SwapChain_Vk::GetBackbufferTextures()
	{
		myBackbufferTextures.Reset();
		uint texCount = 0;
		vkGetSwapchainImagesKHR(myDevice, mySwapChain, &texCount, nullptr);
		myBackbufferTextures.Respace(texCount);
		vkGetSwapchainImagesKHR(myDevice, mySwapChain, &texCount, myBackbufferTextures.GetBuffer());


		for (uint i = 0, count = ourNumBackbuffers; i < count; ++i)
		{
			SR_TextureBufferDesc textureBufDesc;

			textureBufDesc.myWidth = myResolution.x;
			textureBufDesc.myHeight = myResolution.y;
			textureBufDesc.myMips = 1;
			textureBufDesc.myFormat = myFormat;
			textureBufDesc.myFlags = 0;
			textureBufDesc.myDimension = SR_Dimension_Texture2D;

			SC_Ref<SR_TextureBuffer_Vk> texBuf = new SR_TextureBuffer_Vk();
			texBuf->myVkImage = myBackbufferTextures[i];
			texBuf->myProperties = textureBufDesc;

			SR_TextureDesc textureDesc;
			textureDesc.myFormat = textureBufDesc.myFormat;
			textureDesc.myMipLevels = textureBufDesc.myMips;

			myScreenTextures[i] = SR_GraphicsDevice::GetDevice()->CreateTexture(textureDesc, texBuf);

			SR_RenderTargetDesc rtDesc;
			rtDesc.myFormat = textureBufDesc.myFormat;
			myScreenTargets[i] = SR_GraphicsDevice::GetDevice()->CreateRenderTarget(rtDesc, texBuf);
		}
	}

}

#endif