#pragma once

#include <vulkan\vulkan.h>

class VulkanImpl
{
private: //Properties
	VkInstance vkInstance;

public:  // Konstruktor/Destruktor
	VulkanImpl();
	~VulkanImpl();
	
private: // Private Methoden
	
	void initVulkan();
	void destroyVulkan();

};