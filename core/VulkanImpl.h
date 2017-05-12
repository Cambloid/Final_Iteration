#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>

#define IMGUI_MAX_POSSIBLE_BACK_BUFFERS 16
#define IMGUI_VK_QUEUED_FRAMES 2

class VulkanImpl
{
private: 
	//Properties
	HWND hWindow;

	//Framebuffersize -> Default settings
	uint32_t width = 1280;
	uint32_t height = 720;

	//Vulkan Properties
	VkInstance vkInstance = VK_NULL_HANDLE;
	VkAllocationCallbacks *vkAllocationCallbacks = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	uint32_t queueFamilyIdx = 0;

	VkFormat vkImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkFormat vkViewFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkColorSpaceKHR vkColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	//VkImageSubresourceRange  vkImageRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VkDevice vkDevice = VK_NULL_HANDLE;
	VkQueue vkQueue = VK_NULL_HANDLE;

	uint32_t vkBackBufferIndex = 0;
	uint32_t vkBackBufferCount = 0;
	VkImage vkBackBuffer[IMGUI_MAX_POSSIBLE_BACK_BUFFERS] = {};
	VkImageView vkBackBufferView[IMGUI_MAX_POSSIBLE_BACK_BUFFERS] = {};
	VkFramebuffer vkFramebuffer[IMGUI_MAX_POSSIBLE_BACK_BUFFERS] = {};

	uint32_t vkFrameIndex = 0;
	VkCommandPool vkCommandPool[IMGUI_VK_QUEUED_FRAMES];
	VkCommandBuffer vkCommandBuffer[IMGUI_VK_QUEUED_FRAMES];
	VkFence vkFence[IMGUI_VK_QUEUED_FRAMES];
	VkSemaphore vkSemaphore[IMGUI_VK_QUEUED_FRAMES];

	VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

	VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
	VkRenderPass vkRenderPass  = VK_NULL_HANDLE;


public:  // Konstruktor/Destruktor
	VulkanImpl() {};
	~VulkanImpl();

public: // Public Methoden
	void ResizeFramebuffer(int width, int height);
	void Init(HWND hWindow, int width, int height);

private: // Private Methoden
	
	void initVulkan();
	void destroyVulkan();
	void check_vk_result(VkResult err);
	std::vector<char> loadShader(const std::string &filename);

};