
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


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);



class VulkanBase
{
public:
	void Run();

public:
	//窗口大小是否改变
	bool framebufferResized = false;

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
	//管线部局，用于向shader传递uniform变量
	VkPipelineLayout pipelineLayout;
	//管线部局描述符
	VkDescriptorSetLayout descriptorSetLayout;
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

	//顶点数据缓冲对象
	VkBuffer vertexBuffer;
	//顶点数据显存区域
	VkDeviceMemory vertexBufferMemory;
	//索引缓冲对象
	VkBuffer indexBuffer;
	//索引缓冲内存区域
	VkDeviceMemory indexBufferMemory;

	//用于传递到shader中的unifrom数据buffer
	std::vector<VkBuffer> uniformBuffers;
	//用于传递到shader中的unifrom数据内存
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	//描述符池
	VkDescriptorPool descriptorPool;
	//描述符集
	std::vector<VkDescriptorSet> descriptorSets;	
	//纹理数据
	VkImage textureImage;
	//纹理数据的显存
	VkDeviceMemory textureImageMemory;
	//纹理视图
	VkImageView textureImageView;
	//纹理采样器
	VkSampler textureSampler;

	//深度图
	VkImage depthImage;
	//深度图显存
	VkDeviceMemory depthImageMemory;
	//深度图视图
	VkImageView depthImageView;

	//顶点
	std::vector<Vertex> vertices;
	//索引
	std::vector<uint32_t> indices;

	
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
	//清除交换链
	void cleanupSwapChain();
	//重建交换链
	void recreateSwapChain();
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
	//创建顶点数据
	void createVertexBuffer();
	//创建索引数据
	void createIndexBuffer();
	//创建管线部局描述符
	void createDescriptorSetLayout();
	//创建unifrombuffer
	void createUniformBuffers();
	//创建描述符池
	void createDescriptorPool();
	//创建描述符集
	void createDescriptorSets();
	//创建纹理视图
	void createTextureImageView();
	//创建纹理
	void createTextureImage();
	//创建纹理采样器
	void createTextureSampler();
	//创建深度图资源 
	void createDepthResources();
	void loadModel();


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
	//查找显存类型
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	//创建buffer
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	//复制缓存数据
	void copyBUffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	//更新uniform数据
	void updateUniformBuffer(uint32_t currentImage);
	//工具：创建图像缓冲区
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	//工具：变换图像layout
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	//工具，复制缓存数据到图像（copyBuffer的变种）
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	//工具，开始录制单次命令（copyBuffer等操作的第一步）
	VkCommandBuffer beginSingleTimeCommands();
	//工具，结束录制单次命令（copyBuffer等操作的结束）
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	//工具：创建纹理视图
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	//工具：选择支持的图片格式
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	//工具：选择图片格式
	VkFormat findDepthFormat();
	//工具：是否支持深度图格式
	bool hasStencilComponent(VkFormat format);

};
