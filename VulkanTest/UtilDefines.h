#pragma once

#include <vector>
#include <memory>
#include <fstream>

//可并行处理的帧
const int MAX_FRAMES_IN_FLIGHT = 2;

//定义窗口大小
const int WIDTH = 1280;
const int HEIGHT = 720;

//surface支持交换链的细节
struct SwapChainSupportDetails {
	//基础表面特性
	VkSurfaceCapabilitiesKHR capabilities;
	//表面支持的格式
	std::vector<VkSurfaceFormatKHR> formats;
	//支持的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};

//工具：读取文件内容
static std::vector<char> readFile(const std::string& fileName)
{
	//ate：从文件尾部开始读取
	//binary：以二进制的形式读取文件
	std::unique_ptr<std::ifstream> fl(new std::ifstream(fileName, std::ios::ate | std::ios::binary));
	if (!fl->is_open())
	{
		throw std::runtime_error("读取文件失败");
	}

	//使用ate 模式，从文件尾部开始读取，这样就能通过获取位置的接口获得文件大小
	size_t fileSize = (size_t)fl->tellg();
	std::vector<char> buffer(fileSize);

	//将文件跳回头部，读取整个文件
	fl->seekg(0);
	fl->read(buffer.data(), fileSize);

	fl->close();
	return buffer;
}