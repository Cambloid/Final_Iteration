#include "VulkanImpl.h"


// Konstruktor/Destruktor
VulkanImpl::VulkanImpl()
{

}

VulkanImpl::~VulkanImpl()
{

}

// Private Methoden
void VulkanImpl::initVulkan()
{
	VkInstanceCreateInfo vkInstanceCreateInfo;
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.enabledLayerCount = 0;
	vkInstanceCreateInfo.ppEnabledExtensionNames = nullptr;

	//vkInstanceCreateInfo.flags = nullptr;
}

void VulkanImpl::destroyVulkan()
{

}
