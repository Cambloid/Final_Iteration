#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <cstdlib>
#include <cstdio>

class VulkanImpl
{
private: //Properties
	//GLFW Properties
	GLFWwindow *glfwWindow;

	//Vulkan Properties
	VkInstance vkInstance = VK_NULL_HANDLE;
	VkAllocationCallbacks *vkAllocationCallbacks = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

public:  // Konstruktor/Destruktor
	VulkanImpl();
	~VulkanImpl();
	
private: // Private Methoden
	
	void initVulkan();
	void destroyVulkan();
	void check_vk_result(VkResult err);

};