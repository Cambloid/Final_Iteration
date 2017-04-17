cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS:STRING= /machine:x64)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "cmake/")
set(NAME final_iteration)



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

	include_directories(${Vulkan_INCLUDE_DIRS})
	include_directories(${GLFW_INCLUDE_DIRS})
	
	message(${GLFW_INCLUDE_DIRS})

	file(GLOB SOURCE core/*.cpp core/*.h)
	#add_executable(core WIN32 core/main.cpp)
	add_executable(core WIN32 ${SOURCE})
	target_link_libraries(core ${GLFW_LIBRARIES} ${Vulkan_LIBRARY} ${WINLIBS})