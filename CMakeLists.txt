cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS:STRING= /machine:x64)

#Folders which includes cmake scritps for finding libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "cmake/")
set(NAME final_iteration)

#To find Qt libraries
set (CMAKE_PREFIX_PATH "C:\\Qt\\5.9\\msvc2015_64\\")
	

#FindVulkan.cmake in cmake directory
#Find Vulkan libraries
message(STATUS "Find Vulkan libraries")
find_package(Vulkan)

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Vulkan libraries not found")
ELSE()
	message(STATUS "Vulkan libraries found")
ENDIF()


#Find GLFW3
message(STATUS "Find GLFW libraries")
set(GLFW_ROOT "libraries/glfw")
find_package(GLFW)

IF (NOT GLFW_FOUND)
	message(FATAL_ERROR "GLFW libraries not found")
ELSE()
	message(STATUS "GLFW libraries found")
ENDIF()

#Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)


IF (NOT Qt5Widgets_FOUND)
	message(FATAL_ERROR "Qt5Widgets libraries not found")
ELSE()
	message(STATUS "Qt5Widgets libraries found")
ENDIF()


	include_directories(${Vulkan_INCLUDE_DIRS})
	include_directories(${GLFW_INCLUDE_DIRS})
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	

	file(GLOB SOURCE core/*.cpp core/*.h)
	add_executable(core WIN32 ${SOURCE})
	target_link_libraries(core ${GLFW_LIBRARIES} ${Vulkan_LIBRARY} ${WINLIBS} Qt5::Core Qt5::Widgets)