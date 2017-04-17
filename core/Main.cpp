#include <Windows.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <iostream>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanImpl.h"

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {


	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui Vulkan example", NULL, NULL);

	// Setup Vulkan
	if (!glfwVulkanSupported())
	{
		//printf("GLFW: Vulkan Not Supported\n");
		return 1;
	}


	VulkanImpl impl;
	

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();


	}

	glfwTerminate();

	return 0;
}