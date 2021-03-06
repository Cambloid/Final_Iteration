#include <Windows.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

#include "VulkanImpl.h"

#include <QtWidgets>

int main(int argc, char *argv[]) 
{
	QApplication app(argc, argv);

	QWindow qWindow;
	qWindow.resize(1280, 720);
	qWindow.show();
	qWindow.setTitle("Final Iteration");

	VulkanImpl impl;

	//Init Vulkan
	impl.Init(reinterpret_cast<HWND>(qWindow.winId()), 1280, 720);
	impl.DrawFrame();


	return app.exec();
}