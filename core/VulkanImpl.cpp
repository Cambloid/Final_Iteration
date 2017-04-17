#include "VulkanImpl.h"


// Konstruktor/Destruktor
VulkanImpl::VulkanImpl()
{
	this->initVulkan();
}

VulkanImpl::~VulkanImpl()
{

}

// Private Methoden
void VulkanImpl::initVulkan()
{
	uint32_t glfw_extensions_count;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	VkInstanceCreateInfo vkInstanceCreateInfo = {};
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	
	/*
	* Vulkan Required Extensions
	*
	* 0. VK_KHR_surface
	* 1. VK_KHR_win32_surface
	*/
	vkInstanceCreateInfo.enabledExtensionCount = glfw_extensions_count;
	vkInstanceCreateInfo.ppEnabledExtensionNames = glfw_extensions;
	
	check_vk_result(
		vkCreateInstance(&vkInstanceCreateInfo, vkAllocationCallbacks, &vkInstance)
	);

	// Create Window Surface
	{
		check_vk_result(
			glfwCreateWindowSurface(vkInstance, glfwWindow, vkAllocationCallbacks, &vkSurfaceKHR)
		);
	}



}

void VulkanImpl::destroyVulkan()
{

}

void VulkanImpl::check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}