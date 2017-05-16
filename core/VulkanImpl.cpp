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
		std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		VkInstanceCreateInfo vkInstanceCreateInfo = {};
		vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		/*
		* Vulkan Required Extensions
		* 0. VK_KHR_surface
		* 1. VK_KHR_win32_surface
		*/
		vkInstanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		vkInstanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

		this->check_vk_result(
			vkCreateInstance(&vkInstanceCreateInfo, this->vkAllocationCallbacks, &this->vkInstance)
		);
	}

	// Get GPU (WARNING here we assume the first gpu is one we can use)
	{
		uint32_t count;
		this->check_vk_result(
			vkEnumeratePhysicalDevices(this->vkInstance, &count, VK_NULL_HANDLE)
		);

		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		for (uint32_t i = 0; i < count; i++) {
			this->check_vk_result(
				vkEnumeratePhysicalDevices(vkInstance, &count, &this->vkPhysicalDevice)
			);

			vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &vkPhysicalDeviceProperties);

			std::cout << "GPU :" << i << std::endl;
			std::cout << "Device Name: " << vkPhysicalDeviceProperties.deviceName << std::endl;
			std::cout << "Device Type: " << vkPhysicalDeviceProperties.deviceType << std::endl;
			std::cout << "Device ID  : " << vkPhysicalDeviceProperties.deviceID << std::endl;
			std::cout << "===========================================" << std::endl;

		}

		this->check_vk_result(
			vkEnumeratePhysicalDevices(this->vkInstance, &count, &this->vkPhysicalDevice)
		);
	}

	// Get graphic queue -> Get idx to graphics queue
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

		this->check_vk_result(
			vkCreateDevice(this->vkPhysicalDevice, &create_info, this->vkAllocationCallbacks, &this->vkDevice)
		);

		vkGetDeviceQueue(this->vkDevice, this->queueFamilyIdx, queue_index, &this->vkQueue);
	}

	// Create Window Surface -> with HWND (Windows Specific)
	{
		VkWin32SurfaceCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = this->hWindow;
		createInfo.hinstance = GetModuleHandle(nullptr);

		this->check_vk_result(
			vkCreateWin32SurfaceKHR(this->vkInstance,
				&createInfo,
				this->vkAllocationCallbacks,
				&this->vkSurfaceKHR
			)
		);
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
			{ VK_FORMAT_B8G8R8A8_SRGB,  VK_FORMAT_B8G8R8A8_UNORM }
		};

		uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vkPhysicalDevice, this->vkSurfaceKHR, &count, VK_NULL_HANDLE); // Get Count

		VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);

		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vkPhysicalDevice, this->vkSurfaceKHR, &count, formats); //Fill Data

		for (size_t i = 0; i < sizeof(image_view_format) / sizeof(image_view_format[0]); i++)
		{
			for (uint32_t j = 0; j < count; j++)
			{
				/*
				Select
				vkImageFormat: VK_FORMAT_B8G8R8A8_SRGB
				vkViewFormat:  VK_FORMAT_B8G8R8A8_UNORM
				vkColorSpace:  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
				*/
				if (formats[j].format == image_view_format[i][0])
				{
					this->vkImageFormat = image_view_format[i][0];
					this->vkViewFormat = image_view_format[i][1];
					this->vkColorSpace = formats[j].colorSpace;
				}
			}
		}
		free(formats);
	}

	// Create Swapchain/Framebuffers
	{
		this->ResizeFramebuffer(this->width, this->height);
	}

	// Create Command Buffers
	{
		for (int i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++)
		{
			{
				VkCommandPoolCreateInfo info = {};
				
				info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				info.queueFamilyIndex = this->queueFamilyIdx;

				this->check_vk_result(
					vkCreateCommandPool(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkCommandPool[i])
				);

			}

			{
				VkCommandBufferAllocateInfo info = {};
				
				info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				info.commandPool = this->vkCommandPool[i];
				info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				info.commandBufferCount = IMGUI_VK_QUEUED_FRAMES;
				
				this->check_vk_result(
					vkAllocateCommandBuffers(this->vkDevice, &info, &this->vkCommandBuffer[i])
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

		this->check_vk_result(
			vkCreateDescriptorPool(this->vkDevice, &pool_info, this->vkAllocationCallbacks, &this->vkDescriptorPool)
		);

	}

	// Create graphics Pipeline and Shaders
	{

		// Vertex Shader
		std::vector<char> vertShaderCode = this->loadShader("shaders/vert.spv");

		VkShaderModuleCreateInfo vertCreateInfo = {};
		vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertCreateInfo.codeSize = vertShaderCode.size();
		std::vector<uint32_t> vertCodeAligned(vertShaderCode.size() / 4 + 1);
		memcpy(vertCodeAligned.data(), vertShaderCode.data(), vertShaderCode.size());
		vertCreateInfo.pCode = vertCodeAligned.data();

		VkShaderModule vertShaderModule;
		vkCreateShaderModule(this->vkDevice, &vertCreateInfo, this->vkAllocationCallbacks, &vertShaderModule);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";



		// Fragment Shader
		std::vector<char> fragShaderCode = this->loadShader("shaders/frag.spv");

		VkShaderModuleCreateInfo fragCreateInfo = {};
		fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragCreateInfo.codeSize = fragShaderCode.size();
		std::vector<uint32_t> fragCodeAligned(fragShaderCode.size() / 4 + 1);
		memcpy(fragCodeAligned.data(), fragShaderCode.data(), fragShaderCode.size());
		fragCreateInfo.pCode = fragCodeAligned.data();

		VkShaderModule fragShaderModule;
		vkCreateShaderModule(this->vkDevice, &fragCreateInfo, this->vkAllocationCallbacks, &fragShaderModule);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";



		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


		//Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		//Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		// Viewports and scissors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->width;
		viewport.height = (float)this->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent.width = this->width;
		scissor.extent.height = this->height;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		//Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; /// Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Color Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional


		//Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

		VkPipelineLayout pipelineLayout;

		// TODO: vkDestroyPipelineLayout
		this->check_vk_result(
			vkCreatePipelineLayout(this->vkDevice, &pipelineLayoutInfo, this->vkAllocationCallbacks, &pipelineLayout)
		);


		//Renderpass
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = this->vkImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;


		this->check_vk_result(
			vkCreateRenderPass(this->vkDevice, &renderPassInfo, this->vkAllocationCallbacks, &this->vkRenderPass)
		);


		//Create graphics Pipelin
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = VK_NULL_HANDLE; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = VK_NULL_HANDLE; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = this->vkRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional


		this->check_vk_result(
			vkCreateGraphicsPipelines(this->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, this->vkAllocationCallbacks, &this->vkPipeline)
		);

	}


	// Record Command Buffers
	{
		for (size_t i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++) {
			VkCommandBufferBeginInfo beginInfo = {};

			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			vkBeginCommandBuffer(this->vkCommandBuffer[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = this->vkRenderPass;
			renderPassInfo.framebuffer = this->vkFramebuffer[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent.width = this->width;
			renderPassInfo.renderArea.extent.height = this->height;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(this->vkCommandBuffer[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(this->vkCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkPipeline);
			vkCmdDraw(this->vkCommandBuffer[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(this->vkCommandBuffer[i]);

			this->check_vk_result(
				vkEndCommandBuffer(this->vkCommandBuffer[i])
			);
		}

	}

	// Create Semaphores
	{

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		this->check_vk_result(
			vkCreateSemaphore(this->vkDevice, &semaphoreInfo, this->vkAllocationCallbacks, &this->vkSemImageAvailable)
		);
		this->check_vk_result(
			vkCreateSemaphore(this->vkDevice, &semaphoreInfo, this->vkAllocationCallbacks, &this->vkSemRenderFinished)
		);

	}

}

std::vector<char> VulkanImpl::loadShader(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void VulkanImpl::destroyVulkan()
{
	vkDestroyDescriptorPool(this->vkDevice, this->vkDescriptorPool, this->vkAllocationCallbacks);
	
	for (int i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++)
	{
		vkFreeCommandBuffers(this->vkDevice, this->vkCommandPool[i], 1, &this->vkCommandBuffer[i]);
		vkDestroyCommandPool(this->vkDevice, this->vkCommandPool[i], this->vkAllocationCallbacks);
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
	this->check_vk_result(
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
		
		this->check_vk_result(
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

		this->check_vk_result(
			vkCreateSwapchainKHR(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkSwapchain)
		);

		this->check_vk_result(
			vkGetSwapchainImagesKHR(this->vkDevice, this->vkSwapchain, &this->vkBackBufferCount, VK_NULL_HANDLE)
		);

		this->check_vk_result(
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

		this->check_vk_result(
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

		/*
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		*/
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;


		for (uint32_t i = 0; i < this->vkBackBufferCount; i++)
		{
			info.image = this->vkBackBuffer[i];
			this->check_vk_result(
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
			this->check_vk_result(
				vkCreateFramebuffer(this->vkDevice, &info, this->vkAllocationCallbacks, &this->vkFramebuffer[i])
			);
		}

	}


}

void VulkanImpl::Init(HWND hWindow, int width, int height)
{
	this->width = width;
	this->height = height;
	this->hWindow = hWindow;

	this->initVulkan();
}

void VulkanImpl::DrawFrame() {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(this->vkDevice, this->vkSwapchain, std::numeric_limits<uint64_t>::max(), this->vkSemImageAvailable, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->vkSemImageAvailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->vkCommandBuffer[imageIndex];

	VkSemaphore signalSemaphores[] = { this->vkSemRenderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	this->check_vk_result(
		vkQueueSubmit(this->vkQueue, 1, &submitInfo, VK_NULL_HANDLE)
	);

	VkSwapchainKHR swapChains[] = { this->vkSwapchain };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(this->vkQueue, &presentInfo);
}