#include "VulkanImpl.h"


VulkanImpl::~VulkanImpl()
{
	this->destroyVulkan();
}

// Private Methoden
void VulkanImpl::initVulkan()
{

	// Create Vulkan Instance
	{
		uint32_t glfw_extensions_count;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

		VkInstanceCreateInfo vkInstanceCreateInfo = {};
		vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		/*
		* Vulkan Required Extensions
		* 0. VK_KHR_surface
		* 1. VK_KHR_win32_surface
		*/
		vkInstanceCreateInfo.enabledExtensionCount = glfw_extensions_count;
		vkInstanceCreateInfo.ppEnabledExtensionNames = glfw_extensions;

		check_vk_result(
			vkCreateInstance(&vkInstanceCreateInfo, this->vkAllocationCallbacks, &this->vkInstance)
		);
	}

	// Create Window Surface
	{
		check_vk_result(
			glfwCreateWindowSurface(this->vkInstance, this->window, this->vkAllocationCallbacks, &this->vkSurfaceKHR)
		);
	}

	// Get GPU (WARNING here we assume the first gpu is one we can use)
	{
		uint32_t count;
		check_vk_result(
			vkEnumeratePhysicalDevices(this->vkInstance, &count, VK_NULL_HANDLE)
		);
		
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		for (int i = 0; i < count; i++) {
			check_vk_result(
				vkEnumeratePhysicalDevices(vkInstance, &count, &this->vkPhysicalDevice)
			);

			vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &vkPhysicalDeviceProperties);

			std::cout << "GPU :"		 << i									  << std::endl;
			std::cout << "Device Name: " << vkPhysicalDeviceProperties.deviceName << std::endl;
			std::cout << "Device Type: " << vkPhysicalDeviceProperties.deviceType << std::endl;
			std::cout << "Device ID  : " << vkPhysicalDeviceProperties.deviceID   << std::endl;
			std::cout << "==========================================="			  << std::endl;

		}

		check_vk_result(
			vkEnumeratePhysicalDevices(this->vkInstance, &count, &this->vkPhysicalDevice)
		);
	}

	// Get graphic queue
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDevice, &count, VK_NULL_HANDLE);
		VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
		vkGetPhysicalDeviceQueueFamilyProperties(this->vkPhysicalDevice, &count, queues);
		
		for (uint32_t i = 0; i < count; i++)
		{
			if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				std::cout << "Queue Family for rendering Graphics:" << i << std::endl;
				this->queueFamilyIdx = i;
				break;
			}
		}

		free(queues);
	}

	// Check for WSI support
	{
		VkBool32 res;
		vkGetPhysicalDeviceSurfaceSupportKHR(this->vkPhysicalDevice, this->queueFamilyIdx, this->vkSurfaceKHR, &res);
		if (res != VK_TRUE)
		{
			fprintf(stderr, "Error no WSI support on physical device 0\n");
			exit(-1);
		}
	}

	// Get Surface Format
	{
		VkFormat image_view_format[][2] = {
			{ VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM },
			{ VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM }
		};

		uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vkPhysicalDevice, this->vkSurfaceKHR, &count, NULL);
		VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vkPhysicalDevice, this->vkSurfaceKHR, &count, formats);
		
		for (size_t i = 0; i < sizeof(image_view_format) / sizeof(image_view_format[0]); i++)
		{
			for (uint32_t j = 0; j < count; j++)
			{
				if (formats[j].format == image_view_format[i][0])
				{
					this->vkImageFormat = image_view_format[i][0];
					this->vkViewFormat  = image_view_format[i][1];
					this->vkColorSpace  = formats[j].colorSpace;
				}
			}
		}
		free(formats);
	}

	// Create Logical Device
	{
		int device_extension_count = 1;
		const char* device_extensions[] = { "VK_KHR_swapchain" };
		const uint32_t queue_index = 0;
		const uint32_t queue_count = 1;
		const float queue_priority[] = { 1.0f };

		VkDeviceQueueCreateInfo queue_info[1] = {};
		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].queueFamilyIndex = queueFamilyIdx;
		queue_info[0].queueCount = queue_count;
		queue_info[0].pQueuePriorities = queue_priority;

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
		create_info.pQueueCreateInfos = queue_info;
		create_info.enabledExtensionCount = device_extension_count;
		create_info.ppEnabledExtensionNames = device_extensions;

		check_vk_result(
			vkCreateDevice(this->vkPhysicalDevice, &create_info, this->vkAllocationCallbacks, &this->vkDevice)
		);

		vkGetDeviceQueue(this->vkDevice, this->queueFamilyIdx, queue_index, &this->vkQueue);
	}

	// Create Framebuffers
	{
		this->ResizeFramebuffer(this->width, this->height);

		//glfwGetFramebufferSize(this->glfwWindow, &w, &h);
		//resizeFramebuffer(this->glfwWindow, w, h);


		//glfwSetFramebufferSizeCallback(this->glfwWindow, fun);

	}

	// Create Command Buffers
	{
		for (int i = 0; i < 2; i++)
		{
			{
				VkCommandPoolCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				info.queueFamilyIndex = this->queueFamilyIdx;

				check_vk_result(
					vkCreateCommandPool(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkCommandPool[i])
				);
			}
			{
				VkCommandBufferAllocateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				info.commandPool = this->vkCommandPool[i];
				info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				info.commandBufferCount = 1;
				check_vk_result(
					vkAllocateCommandBuffers(this->vkDevice, &info, &this->vkCommandBuffer[i])
				);
			}
			{
				VkFenceCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				check_vk_result(
					vkCreateFence(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkFence[i])
				);
			}
			{
				VkSemaphoreCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				check_vk_result(
					vkCreateSemaphore(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkSemaphore[i])
				);
			}
		}
	}

	// Create Descriptor Pool
	{
		VkDescriptorPoolSize pool_size[11] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};

		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * 11;
		pool_info.poolSizeCount = 11;
		pool_info.pPoolSizes = pool_size;

		check_vk_result(
			vkCreateDescriptorPool(this->vkDevice, &pool_info, this->vkAllocationCallbacks, &this->vkDescriptorPool)
		);
		
	}

}

void VulkanImpl::destroyVulkan()
{
	vkDestroyDescriptorPool(this->vkDevice, this->vkDescriptorPool, this->vkAllocationCallbacks);
	
	for (int i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++)
	{
		vkDestroyFence(this->vkDevice, this->vkFence[i], this->vkAllocationCallbacks);
		vkFreeCommandBuffers(this->vkDevice, this->vkCommandPool[i], 1, &this->vkCommandBuffer[i]);
		vkDestroyCommandPool(this->vkDevice, this->vkCommandPool[i], this->vkAllocationCallbacks);
		vkDestroySemaphore(this->vkDevice, this->vkSemaphore[i], this->vkAllocationCallbacks);
	}

	for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
	{
		vkDestroyImageView(this->vkDevice, this->vkBackBufferView[i], this->vkAllocationCallbacks);
		vkDestroyFramebuffer(this->vkDevice, this->vkFramebuffer[i], this->vkAllocationCallbacks);
	}

	vkDestroyRenderPass(this->vkDevice, this->vkRenderPass, this->vkAllocationCallbacks);
	vkDestroySwapchainKHR(this->vkDevice, this->vkSwapchain, this->vkAllocationCallbacks);
	vkDestroySurfaceKHR(this->vkInstance, this->vkSurfaceKHR, this->vkAllocationCallbacks);
	vkDestroyDevice(this->vkDevice, this->vkAllocationCallbacks);
	vkDestroyInstance(this->vkInstance, this->vkAllocationCallbacks);
}

void VulkanImpl::check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

//Public Methoden
void VulkanImpl::ResizeFramebuffer(int width, int height)
{
	VkSwapchainKHR old_swapchain = this->vkSwapchain;

	//vkDeviceWaitIdle - Wait for a device to become idle
	check_vk_result(
		vkDeviceWaitIdle(this->vkDevice)
	);

	// Destroy old Framebuffer:
	for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
		if (this->vkBackBufferView[i])
			vkDestroyImageView(this->vkDevice, this->vkBackBufferView[i], this->vkAllocationCallbacks);

	for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
		if (this->vkFramebuffer[i])
			vkDestroyFramebuffer(this->vkDevice, this->vkFramebuffer[i], this->vkAllocationCallbacks);

	if (this->vkRenderPass)
		vkDestroyRenderPass(this->vkDevice, this->vkRenderPass, this->vkAllocationCallbacks);

	// Create Swapchain:
	{
		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = this->vkSurfaceKHR;
		info.imageFormat = this->vkImageFormat;
		info.imageColorSpace = this->vkColorSpace;
		info.imageArrayLayers = 1;
		info.imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		info.clipped = VK_TRUE;
		info.oldSwapchain = old_swapchain;

		VkSurfaceCapabilitiesKHR cap;
		
		check_vk_result(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->vkPhysicalDevice, this->vkSurfaceKHR, &cap)
		);
		
		if (cap.maxImageCount > 0)
			info.minImageCount = (cap.minImageCount + 2 < cap.maxImageCount) ? (cap.minImageCount + 2) : cap.maxImageCount;
		else
			info.minImageCount = cap.minImageCount + 2;

		if (cap.currentExtent.width == 0xffffffff)
		{
			this->width = width;
			this->height = height;
			info.imageExtent.width = this->width;
			info.imageExtent.height = this->height;
		}
		else
		{
			this->width = cap.currentExtent.width;
			this->height = cap.currentExtent.height;
			info.imageExtent.width = this->width;
			info.imageExtent.height = this->height;
		}

		check_vk_result(
			vkCreateSwapchainKHR(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkSwapchain)
		);

		check_vk_result(
			vkGetSwapchainImagesKHR(this->vkDevice, this->vkSwapchain, &this->vkBackBufferCount, NULL)
		);

		check_vk_result(
			vkGetSwapchainImagesKHR(this->vkDevice, this->vkSwapchain, &this->vkBackBufferCount, this->vkBackBuffer)
		);

	}
	if (old_swapchain)
		vkDestroySwapchainKHR(this->vkDevice, old_swapchain, this->vkAllocationCallbacks);

	// Create the Render Pass:
	{
		VkAttachmentDescription attachment = {};
		attachment.format = this->vkViewFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		check_vk_result(
			vkCreateRenderPass(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkRenderPass)
		);
	}

	// Create The Image Views
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = this->vkViewFormat;
		info.components.r = VK_COMPONENT_SWIZZLE_R;
		info.components.g = VK_COMPONENT_SWIZZLE_G;
		info.components.b = VK_COMPONENT_SWIZZLE_B;
		info.components.a = VK_COMPONENT_SWIZZLE_A;
		info.subresourceRange = this->vkImageRange;
		for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
		{
			info.image = this->vkBackBuffer[i];
			check_vk_result(
				vkCreateImageView(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkBackBufferView[i])
			);
		}
	}

	// Create Framebuffer:
	{
		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = this->vkRenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.width = this->width;
		info.height = this->height;
		info.layers = 1;

		for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
		{
			attachment[0] = this->vkBackBufferView[i];
			check_vk_result(
				vkCreateFramebuffer(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkFramebuffer[i])
			);
		}
	}


}

void VulkanImpl::Init(GLFWwindow* window,  int width, int height)
{
	this->width = width;
	this->height = height;
	this->window = window;

	this->initVulkan();
}
