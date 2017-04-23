#include <Windows.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanImpl.h"

#include <QtWidgets>

VulkanImpl impl;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void resizeFrameBuffer(GLFWwindow *window, int width, int heigth) {
	impl.ResizeFramebuffer(width, heigth);
}


int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) return 1;

	int w = 1280;
	int h = 720;

	QMainWindow qwindow;
	qwindow.show();
	

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(w, h, "Final Iteration", NULL, NULL);


	//Init Vulkan
	impl.Init(window, w, h);

	glfwSetFramebufferSizeCallback(window, resizeFrameBuffer);

	// Setup Vulkan
	if (!glfwVulkanSupported())
	{
		//printf("GLFW: Vulkan Not Supported\n");
		return 1;
	}

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}