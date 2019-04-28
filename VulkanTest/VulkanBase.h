
#pragma once
#define GLFW_INCLUDE_VULKAN


#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <memory>
#include "UtilDefines.h"



//列队族查找结果
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//查询到的交换链细节信息
struct SwapChainSupportDetails {
	//基础表面特性
	VkSurfaceCapabilitiesKHR capabilities;
	//表面支持的格式
	std::vector<VkSurfaceFormatKHR> formats;
	//支持的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);



class VulkanBase
{
public:
	void Run();

private:
	GLFWwindow* window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	//窗口表面
	VkSurfaceKHR surface;

	//物理设备
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//逻辑设备
	VkDevice device;

	//队列
	//图形队列 
	VkQueue graphicsQueue;
	//呈现队列
	VkQueue presentQueue;

	//交换链
	VkSwapchainKHR swapChain;
	//交换链图像
	std::vector<VkImage> swapChainImages;
	//交换链表面格式
	VkFormat swapChainImageFormat;
	//交换链范围
	VkExtent2D swapChainExtent;
	//图像视图
	std::vector<VkImageView> swapChainImageViews;
	//渲染流程
	VkRenderPass renderPass;
	//管线部局
	VkPipelineLayout pipelineLayout;
	//图形渲染管线
	VkPipeline graphicsPipeline;
	//帧缓冲
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	//命令池
	VkCommandPool commandPool;
	//命令缓冲，命令缓冲对象会在命令缓冲池被清除时自动被清除
	std::vector<VkCommandBuffer> commandBuffers;

	//以下两个信号量用于渲染和显示，一个通知图像已被获取可以开始渲染，另一个通知渲染已经结束可以开始显示
	//图像已被获取的信号量
	std::vector<VkSemaphore> imageAvailableSemaphores;
	//通知渲染已经结束的信号量
	std::vector<VkSemaphore> renderFinishedSemaphores;
	//光栅对象,用于CPU 和GPU之间的同步，防止有超过MAX_FRAMES_IN_FLIGHT帧的指令同时被提交执行
	std::vector<VkFence> inFlightFences;
	//当前渲染的是那一帧
	size_t currentFrame = 0;

private :

	//初始化窗口
	void initWindow();
	//初始化
	void initVulkan();
	//主程序循环
	void mainLoop();
	//清理内存
	void cleanup();
	//创建vulkan实例
	void createInstance();
	//设置校验层
	void setupDebugMessenger();		
	//创建窗口表面
	void createSurface();
	//选择物理设备和队列族
	void pickPhysicalDevice();
	//逻辑设备和队列
	void createLogicalDevice();
	//建立交换链
	void createSwapChain();
	//建立图像视图
	void createImageViews();
	//建立渲染Pass
	void createRenderPass();
	//建立图形管线
	void createGraphicsPipeline();
	//建立帧缓冲
	void createFramebuffers();
	//建立命令池
	void createCommandPool();
	//建立命令缓冲
	void createCommandBuffer();
	//建立同步对象
	void createSyncObjects();
	//绘制
	void drawFrame();



	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	//创建shader模块
	VkShaderModule createShaderModule(const std::vector<char>& code);

	//校验层回调，在此消息内处理vulkan回调的错误码
	//使用VKAPI_ATTR 和VKAPI_CALL 定义,这样才能被Vulkan库调用
	// messageSeverity 为消息级别，它可以这些值 ：
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT：诊断信息
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT：	资源创建之类的信息
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT：		警告信息
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT：		不合法和可能造成崩溃的操作信息
	//messageType 参数可以是下面这些值：
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT：	发生了一些与规范和性能无关的事件
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT：	出现了违反规范的情况或发生了一个可能的错误
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT：	进行了可能影响Vulkan 性能的行为
	//pCallbackData参数是一个指向VkDebugUtilsMessengerCallbackDataEXT结构体的指针,包含：pMessage：一个以null 结尾的包含调试信息的字符串；pObjects：存储有和消息相关的Vulkan 对象句柄的数组；objectCount：数组中的对象个数
	//pUserData是一个指向了我们设置回调函数时，传递的数据的指针
	//返回了一个布尔值，用来表示引发校验层处理的Vulkan API调用是否被中断。如果返回值为true，对应Vulkan API 调用就会返回VK_ERROR_VALIDATION_FAILED_EXT 错误代码。通常，只在测试校验层本身时会返回true，其余情况下，回调函数应该返回VK_FALSE。
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			//todo::此消息为重要消息，需要向上层显示
		}

		return VK_FALSE;
	}
};
