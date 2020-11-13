#pragma once
#include "SR_RenderDefines.h"

#if ENABLE_VULKAN

#define SR_VK_FORWARD_DECL(aType) typedef struct aType##_T *aType

SR_VK_FORWARD_DECL(VkInstance);
SR_VK_FORWARD_DECL(VkDevice);
SR_VK_FORWARD_DECL(VkPhysicalDevice);
SR_VK_FORWARD_DECL(VkCommandBuffer);
SR_VK_FORWARD_DECL(VkImage);
SR_VK_FORWARD_DECL(VkImageView);
SR_VK_FORWARD_DECL(VkBuffer);
SR_VK_FORWARD_DECL(VkObject);
SR_VK_FORWARD_DECL(VkQueue);
SR_VK_FORWARD_DECL(VkSemaphore);
SR_VK_FORWARD_DECL(VkFence);
SR_VK_FORWARD_DECL(VkDeviceMemory);
SR_VK_FORWARD_DECL(VkEvent);
SR_VK_FORWARD_DECL(VkQueryPool);
SR_VK_FORWARD_DECL(VkBufferView);
SR_VK_FORWARD_DECL(VkImageView);
SR_VK_FORWARD_DECL(VkShaderModule);
SR_VK_FORWARD_DECL(VkPipelineCache);
SR_VK_FORWARD_DECL(VkPipelineLayout);
SR_VK_FORWARD_DECL(VkRenderPass);
SR_VK_FORWARD_DECL(VkPipeline);
SR_VK_FORWARD_DECL(VkDescriptorSetLayout);
SR_VK_FORWARD_DECL(VkSampler);
SR_VK_FORWARD_DECL(VkDescriptorPool);
SR_VK_FORWARD_DECL(VkDescriptorSet);
SR_VK_FORWARD_DECL(VkFramebuffer);

// Debug
SR_VK_FORWARD_DECL(VkDebugUtilsMessengerEXT);

// Presentation
SR_VK_FORWARD_DECL(VkSurfaceKHR);
SR_VK_FORWARD_DECL(VkSwapchainKHR);

#undef SR_VK_FORWARD_DECL

#endif