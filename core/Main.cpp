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

/*
int CALLBACK WinMain(
_In_ HINSTANCE hInstance,
_In_ HINSTANCE hPrevInstance,
_In_ LPSTR     lpCmdLine,
_In_ int       nCmdShow
)
*/

int main(int argc, char *argv[]) 
{
	QApplication app(argc, argv);

	QWindow qWindow;
	

	qWindow.resize(1280, 720);
	qWindow.show();
	//qWindow.setWindowTitle(QApplication::translate("toplevel", "Top-level widget"));


	//Init Vulkan
	impl.Init(reinterpret_cast<HWND>(qWindow.winId()), 1280, 720);
	
	return app.exec();
}