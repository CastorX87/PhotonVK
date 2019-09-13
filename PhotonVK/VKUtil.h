#pragma once
#ifndef _VKUTIL_H_
#define _VKUTIL_H_

#define STL_CONTAINS(collection,elemToFind) (std::find((collection).begin(), (collection).end(), elemToFind) != (collection).end())

#define REGISTER_OBJ_NAME(var, VkObjType, objTypeEnum) {(var).setDebugUtilsObjectNameEXT(vk::DebugUtilsObjectNameInfoEXT(objTypeEnum, reinterpret_cast<uint64_t>((VkObjType)vkDevice), #var), vkDispatcher);}

#define PRINT(x)              { std::cout << white  << (x) << std::endl; }
#define PRINT_APP_COMMENT(x)  { std::cout << green  << (x) << std::endl; }
#define PRINT_APP_INFO(x)     { std::cout << white  << "[PhotonVK:I] " << (x) << std::endl; }
#define PRINT_APP_WARNING(x)  { std::cout << yellow << "[PhotonVK:W] " << (x) << std::endl; }
#define PRINT_APP_ERROR(x)    { std::cout << red    << "[PhotonVK:E] " << (x) << std::endl; }
#define PRINT_VK_VERBOSE(x)   { std::cout << grey   << "[Vulkan:V] " << (x) << std::endl; }
#define PRINT_VK_INFO(x)      { std::cout << white  << "[Vulkan:I] " << (x) << std::endl; }
#define PRINT_VK_WARNING(x)   { std::cout << yellow << "[Vulkan:W] " << (x) << std::endl; }
#define PRINT_VK_ERROR(x)     { std::cout << red    << "[Vulkan:E] " << (x) << std::endl; }


#define DEBUG_PRINT_VECTOR_DATA(msg, x) { std::cout << green  << std::string(msg) + ": "; for(const auto& elem : x) { std::cout << elem << ","; } std::cout << std::endl; }

#define SAFE_DELETE(x) { if((x) != nullptr) { delete (x); (x) = nullptr; } }

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ctime>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <optional>
#include <set>
#include <memory>
#include <algorithm>
#include <set>
#include <map>

#include <vulkan/vulkan.hpp>


#pragma once
#include <iostream>
#include <windows.h>

inline std::ostream& blue(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& dark_blue(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE);
	return s;
}

inline std::ostream& red(std::ostream & s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& dark_red(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED);
	return s;
}

inline std::ostream& green(std::ostream & s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& dark_green(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN);
	return s;
}

inline std::ostream& yellow(std::ostream & s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& grey(std::ostream & s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}

inline std::ostream& white(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	return s;
}

struct color
{
	color(WORD attribute) :m_color(attribute) {};
	WORD m_color;
};

template <class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>&
operator << (std::basic_ostream<_Elem, _Traits> & i, color & c)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, c.m_color);
	return i;
}

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

#endif