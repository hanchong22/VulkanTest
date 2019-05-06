
#include "VulkanBase.h"

//stb的头文件中都没有加#pragma once，所以只能在cpp引用，否则就可能出现重复引用的LNK2005错误
#define STB_IMAGE_IMPLEMENTATION	//开启std库中的图片工具
#include <stb_image.h>				//读取图片工具	


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif




//窗口大小改变时的回调
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


#pragma region 生命周期

void VulkanBase::Run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VulkanBase::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//允许窗口大小改变
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	//把当前对象传入作为用户自定义对象，以便在回调方法中找到当前VulkanBase实例
	glfwSetWindowUserPointer(window, this);
	//窗口大小改变时的回调
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}



void VulkanBase::initVulkan() {
	createInstance();		//创建实例
	setupDebugMessenger();	//设置校验层
	createSurface();		//创建窗口表面
	pickPhysicalDevice();	//选择物理设备和队列族
	createLogicalDevice();	//逻辑设备和队列
	createSwapChain();		//创建交换链
	createImageViews();		//创建交换链图像视图
	createRenderPass();		//创建渲染pass
	createDescriptorSetLayout();	//创建管线部局描述符
	createGraphicsPipeline();	//创建图形管线
	createFramebuffers();		//创建帧缓冲
	createCommandPool();		//创建创建命令池
	createTextureImage();		//创建纹理
	createTextureImageView();	//创建纹理视图
	createTextureSampler();		//创建采样器
	createVertexBuffer();		//创建顶点数据缓冲
	createIndexBuffer();		//创建索引数据缓冲
	createUniformBuffers();		//创建unifrom缓冲
	createDescriptorPool();		//创建描述符池
	createDescriptorSets();		//创建描述会集
	createCommandBuffer();		//建立命令缓冲
	createSyncObjects();		//建立同步对象
}

//主循环
void VulkanBase::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}


void VulkanBase::cleanup() {	

	cleanupSwapChain();

	//顶点缓冲不依赖交换链
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);

	vkDestroyImage(device, textureImage,nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}	
	

	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}
#pragma endregion


#pragma region 实例创建与校验层设置

//创建实例
void VulkanBase::createInstance() {

	//检查校验层
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("没有找到校验层!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//校验层扩展
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//校验层
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}



//设置校验层消息处理
void VulkanBase::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//messageSeverity设置回调级别
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//messageType消息类型
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	//回调方法
	createInfo.pfnUserCallback = debugCallback;
	//可选项，回调时的参数
	createInfo.pUserData = nullptr;

	//创建vkCreateDebugUtilsMessengerEXT
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}



//根据是否启用校验层，返回所需的扩展列表
std::vector<const char*> VulkanBase::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

//检查所有可用的校验层
bool VulkanBase::checkValidationLayerSupport() {

	//所有可用的校验层列
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	//检查所有的校验层是否能找到
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

//vkCreateDebugUtilsMessengerEXT创建的代理函数
//由于vkCreateDebugUtilsMessengerEXT 函数是一个扩展函数,不会被Vulkan 库自动加载，所以需要我们自己使用vkGetInstanceProcAddr 函数来加载它
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
#pragma endregion


#pragma region 物理设备和队列族与逻辑队列
//选择物理设备和队列族
void VulkanBase::pickPhysicalDevice() {
	//第一次调用，获得物理设备数量，以便初始化数组数量。
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	//得到数量后进行第二次调用，返回可用的物理设备
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//选择第一个能满足需求的显卡
	//todo:: 给每个设备打分，选择分数最高的设备
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

//返回物理设备是否能满足需求
bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	//获取物理设备支持的特性信息
	//需要支持采样各向异性过滤
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanBase::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}



//建立逻辑设备与队列
void VulkanBase::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	//纹理采样各向异性支持
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	//获取指定队列族的队列句柄
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

//查找满足需求的队列族
QueueFamilyIndices VulkanBase::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	//首先获取列队族个数，然后根据个数来分配对象，再查找出所有列队族
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	//遍历所有列队族，标记支持VK_QUEUE_GRAPHICS_BIT和Physical的列队族
	//找到第一个满足两者的列队族
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		//满足两者
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}
#pragma endregion

#pragma region 窗口表面
//创建窗口表面
void VulkanBase::createSurface() {

	//调用glfw来完成VKsurface创建，这需要glfw的支持，3.0版本的glfw已经支持vulkan
	//此操作将窗口与vulkan实例联系起来
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}	
}
#pragma endregion

#pragma region 交换链

//建立交换链
void VulkanBase::createSwapChain() {
	//交换链细节信息
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	//选择表面格式
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//呈现模式
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//交换范围
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//使用交换链支持的最小图像个数 + 1 数量的图像来实现三倍缓冲
	//maxImageCount 的值为0 表明，只要内存可以满足，我们可以使用任意数量的图像。
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	//每个图像所包含的层次，一般为1，VR会更多
	createInfo.imageArrayLayers = 1;
	//在图像上的操作,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT表示图像可作为传输目的图像
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//指定多个队列族使用交换链图像的方式,对于图形队列和呈现队列不是同一个队列的情况有着很大影响,通过图形队列在交换链图像上进行绘制操作，然后将图像提交给呈现队列来显示
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//VK_SHARING_MODE_EXCLUSIVE： 独占模式，一张图像同一时间只间只能被一个队列族所拥有，在另一队列族使用它之前，必须显式地改变图像所有权。这一模式下性能表现最佳
	//VK_SHARING_MODE_CONCURRENT：协同模式，图像可以在多个队列族间使用，不需要显式地改变图像所有权。	
	if (indices.graphicsFamily != indices.presentFamily) {
		//如果图形和呈现不是同一个队列族（极少情况），我们使用协同模式来避免处理图像所有权问题。协同模式需要我们使用queueFamilyIndexCount 和pQueueFamilyIndices 来指定共享所有权的队列族
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		//如果图形队列族和呈现队列族是同一个队列族(大部分情况下都是这样)，我们就不能使用协同模式，协同模式需要我们指定至少两个不同的队列族。
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	//为交换链中的图像指定一个固定的Transfrom变化操作
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//指定alpha 通道是否被用来和窗口系统中的其它窗口进行混合操作,这里将其设置为VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR来忽略掉alpha 通道。
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//指定前面选好的显示模式
	createInfo.presentMode = presentMode;
	//clipped 成员变量被设置为VK_TRUE 表示我们不关心被窗口系统中的其它窗口遮挡的像素的颜色，这允许Vulkan 采取一定的优化措施，但如果我们回读窗口的像素值就可能出现问题
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

//清除交换链
void VulkanBase::cleanupSwapChain()
{
	for (auto frameBuffer : swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}
	swapChainFrameBuffers.clear();

	//commandBuffers不需要重建，只需调用vkFreeCommandBuffers清除分配的指令缓冲对象
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	//commandBuffers.clear();
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);



	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	swapChainImageViews.clear();

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanBase::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandBuffer();
}

//选择交换链表面格式
//选择支持RGB和SRGB格式
VkSurfaceFormatKHR VulkanBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//优先选择SRGB
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	//默认返回第1个
	return availableFormats[0];
}

//选择呈现模式
VkPresentModeKHR VulkanBase::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

	//呈现模式：
	//VK_PRESENT_MODE_IMMEDIATE_KHR：应用程序提交的图	像会被立即传输到屏幕上，可能会导致撕裂现象。
	//VK_PRESENT_MODE_FIFO_KHR：交换链变成一个先进先出的队列，每次从队列头部取出一张图像进行显示，
	//--------应用程序渲染的图像提交给交换链后，会被放在队列尾部。当队列为满时，应用程序需要进行等待。这一模式非常类似现在常用的垂直同步。刷新显示的时刻也被叫做垂直回扫。
	//VK_PRESENT_MODE_FIFO_RELAXED_KHR：这一模式和上个模式的唯一区别是，如果应用程序延迟，导致交换链的队列在上一次垂直回扫时为空，那么，如果应用程序在下一次垂直回扫前提交图像，图像会立即被显示。这一模式可能会导致撕裂现象。
	//VK_PRESENT_MODE_MAILBOX_KHR：这一模式是第二种模的另一个变种。它不会在交换链的队列满时阻塞应用程序，队列中的图像会被直接替换为应用程序新提交的图像。这一模式可以用来实现三倍缓冲，避免撕裂现象的同时减小了延迟问题。

	//上面四种呈现模式，只有VK_PRESENT_MODE_FIFO_KHR 模式保证一定可用，所以需要编程来选择一种最佳模式(VK_PRESENT_MODE_MAILBOX_KHR优先)
	//其它模式都不可用时才使用此模式
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;//最优模式
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;//次优模式
		}
	}

	return bestMode;
}

//选择交换范围
//交换范围是交换链中图像的分辨率，它几乎总是和我们要显示图像的窗口的分辨率相同
VkExtent2D VulkanBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { width, height };
		

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

//查询交换链细节信息
SwapChainSupportDetails VulkanBase::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	//查询交换链窗口表面特性
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	//查询表面支持的格式
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//查询支持的呈现模式
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


//建立VkImageView，用于访问交换链中的图象
void VulkanBase::createImageViews() {
	//分配足够的数组空间来存储图像视图
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
	}

}
#pragma endregion

#pragma region 图形管线与shader

//创建渲染pass
void VulkanBase::createRenderPass()
{
	//缓冲附件，用于交换链图像的颜色缓冲
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//设置渲染前操作
	//K_ATTACHMENT_LOAD_OP_LOAD：保持附件的现有内容
	//VK_ATTACHMENT_LOAD_OP_CLEAR：使用一个常量值来清除附件的内容
	//VK_ATTACHMENT_LOAD_OP_DONT_CARE：不关心附件现存的内容
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//每次渲染新的一帧前使用黑色清除帧缓冲
	//设置渲染后操作
	//VK_ATTACHMENT_STORE_OP_STORE：渲染的内容会被存储起来，以便之后读取
	//VK_ATTACHMENT_STORE_OP_DONT_CARE：渲染后，不会读取帧缓冲的内容
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//设置模板缓冲
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//设置渲染前后的图像格式
	//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL：图像被用作颜色附件
	//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR：图像被用在交换链中进行呈现操作
	//VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL： 用于作为复制操作的来源(vkCmdCopyImageToBuffer)
	//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL：图像被用作复制操作的目的图像(如vkCmdCopyBufferToImage)
	//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL：	仅用于在shader中读取
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//不关心之前的图像布局方式
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//渲染后的图像可被交换链呈现

	//附件引用
	VkAttachmentReference colorAttchmentRef = {};
	//attachment为附件索引号
	colorAttchmentRef.attachment = 0;
	//布局方式
	colorAttchmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	

	//渲染子pass
	VkSubpassDescription subpass = {};
	//pass类型为图形渲染（非计算）
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//指定颜色附件
	subpass.colorAttachmentCount = 1;
	//pColorAttachments对应fragment shader中的layout(location = 0) out vec4 outColor
	//pInputAttachments：被着色器读取的附件
	//pResolveAttachments：用于多重采样的颜色附件
	//pDepthStencilAttachment：用于深度和模板数据的附件
	//pPreserveAttachments：没有被这一子流程使用，但需要保留数据的附件
	subpass.pColorAttachments = &colorAttchmentRef;

	//渲染pass
	//渲染pass createinfo
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	

	//子pass的依赖
	//子pass需要依赖图形，可以设置等待信号量，也可以设置等待管线阶段
	VkSubpassDependency dependency = {};
	//渲染流程开始前的子pass
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;		//被依赖的子流程的索引,VK_SUBPASS_EXTERNAL为隐含的子流程
	//渲染流程结束后的子pass,索引0是前面创建的子pass的索引，为了避免循环依赖,dstSupass必须大于srcSubpass
	dependency.dstSubpass = 0;							//依赖被依赖的子流程的索引
	//srcStageMask 和srcAccessMask 成员变量用于指定需要等待的管线阶段和子流程将进行的操作类型。
	//我们需要等待交换链结束对图像的读取才能对图像进行访问操作，也就是等待颜色附件输出这一管线阶段。
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	//dstStageMask 和dstAccessMask 成员变量用于指定需要等待的管线阶段和子流程将进行的操作类型。在这里，我们的设置为等待颜色附着的输出阶段，
	//子流程将会进行颜色附着的读写操作。这样设置后，图像布局变换直到必要时才会进行：当我们开始写入颜色数据时。
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("创建渲染pass失败");
	}
}


//创建图形管线、加载shader字节编码（编译后的shader）
void VulkanBase::createGraphicsPipeline() {

	//读取shader字节（需读取编译后的shader，先使用glslangValidator.exe编译shader代码）
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");
	//创建shader模块（字节封装）
	auto vertShader = createShaderModule(vertShaderCode);
	auto fragShader = createShaderModule(fragShaderCode);

	//创建shader阶段，这里确定了shader的类型(frag或vectex)、执行函数
	//创建vectexshader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;//顶点着色器
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName = "main";
	//以下代码可以向shader传递define，这些define一般用于shader内部的分支#if #else #endif
	//vertShaderStageInfo.pSpecializationInfo = nullptr;

	//创建fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;//片元着色器
	fragShaderStageInfo.module = fragShader;
	fragShaderStageInfo.pName = "main";
	//向shader传递的预编译定义
	//fragShaderStageInfo.pSpecializationInfo = nullptr;

	std::vector< VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo ,fragShaderStageInfo };	


	//顶点输入
	//todo::读取网格(mesh)文件传入，此demo中没有读取mesh文件，是因为在shader代码中定义了顶点数据。
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//输入Assembly，即顶点数据类型
	//Assembly用于指定顶点数据定义了哪种类型的几何图元，以及是否启用几何图元重复利用
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//几何图元的类型
	//VK_PRIMITIVE_TOPOLOGY_POINT_LIST：点图元
	//VK_PRIMITIVE_TOPOLOGY_LINE_LIST：每两个顶点构成一个线段图元
	//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP：每两个顶点构成一个线段图元，除第一个线段图元外，每个线段图元使用上一个线段图元的一个顶点
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST：每三个顶点构成一个三角形图元
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP：每个三角形的第二个和第三个顶点被下一个三角形作为第一和第二个顶点使用
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//是否启用几何图元重复利用，是指顶点可以重复利用在多个地方，以节省mesh数据尺寸
	//如果需要重复利用顶点数据，可通过特殊索引值 0xFFFF	或0xFFFFFFFF将索引重新定义到顶点数组的第一个下标
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//视口Viewport和裁剪Rect
	//Viewport，用于定义输出渲染结果的帧缓冲区，此demo中会和窗口一样大
	VkViewport viewPort = {};
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = (float)swapChainExtent.width;
	viewPort.height = (float)swapChainExtent.height;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	//裁剪
	//此demo中和视口一样大
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = swapChainExtent;

	//创建视口和裁剪
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewPort;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//光栅化定义 Rasterization	
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//depthClampEnable 设置为VK_TRUE 表示在近平面和远平面外的片元会被截断为在近平面和远平面上，而不是直接丢弃这些片元。这对于阴影贴图的生成很有用
	rasterizer.depthClampEnable = VK_FALSE;
	//rasterizerDiscardEnable 设置为VK_TRUE 表示所有几何图元都不能通过光栅化阶段。这一设置会禁止一切片元输出到帧缓冲。
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	//polygonMode 指定几何图元生成片段的方式
		//VK_POLYGON_MODE_FILL：整个多边形，包括多边形内部都产生片段
		//VK_POLYGON_MODE_LINE：只有多边形的边会产生片段
		//VK_POLYGON_MODE_POINT：只有多边形的顶点会产生片段
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	//lineWidth指定光栅化后的线段宽度，它以线宽所占的片段数目为单位。线宽的最大值依赖于硬件，使用大于1.0f 的线宽，需要启用相应的GPU 特性。
	rasterizer.lineWidth = 1.0f;
	//cullMode指定使用的表面剔除类型,可以通过它禁用表面剔除，剔除背面，剔除正面，以及剔除双面
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//frontFace于指定顺时针的顶点序是正面，还是逆时针的顶点序是正面
	//rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//可选
	rasterizer.depthBiasClamp = 0.0f;			//可选
	rasterizer.depthBiasSlopeFactor = 0.0f;		//可选


	//多重采样(msaa)配置，常用于抗锯齿，需要启动相应的GPU特性
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//禁用多重采样
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;				//可选
	multisampling.pSampleMask = nullptr;				//可选
	multisampling.alphaToCoverageEnable = VK_FALSE;		//可选
	multisampling.alphaToOneEnable = VK_FALSE;			//可选

	//深度和模板测试
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	

	//两个颜色混合设置

	//VkPipelineColorBlendAttachmentState对每个绑定的帧缓冲进行单独的颜色混合配置	
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	//启用颜色混合,以达到半透明的效果
	colorBlendAttachment.blendEnable = VK_TRUE;

	//VkPipelineColorBlendStateCreateInfo全局的颜色混合配置
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//禁用位运算的混合方式，采用colorBlendAttachment中定义的颜色混合方式，如果启动logicOpEnable，则所有的VkPipelineColorBlendAttachmentState都会被禁用
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;				//可选
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;					//可选
	colorBlending.blendConstants[1] = 0.0f;					//可选
	colorBlending.blendConstants[2] = 0.0f;					//可选
	colorBlending.blendConstants[3] = 0.0f;					//可选

	//动态状态
	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_LINE_WIDTH};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;


	//管线布局
	//管理部局可以用于设置shader中的uniform变量	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;						//描述符集布局数量
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;		//描述符集布局
	pipelineLayoutInfo.pushConstantRangeCount = 0;				//可选
	pipelineLayoutInfo.pPushConstantRanges = nullptr;			//可选

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("创建管线部局失败");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;				//可选
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;					//动态状态，可选

	pipelineInfo.layout = pipelineLayout;				//管线部局
	pipelineInfo.renderPass = renderPass;				//渲染pass
	//子sub索引
	pipelineInfo.subpass = 0;
	//指定继承的其它已经存在的管线（可选）
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		//可选
	//指定的将要创建的管线为基管线（可选）
	pipelineInfo.basePipelineIndex = -1;					//可选

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("创建图形渲染管线失败");
	}
	
	shaderStages.clear();
	//用完后清除shader模块(字节封装)
	vkDestroyShaderModule(device, fragShader, nullptr);
	vkDestroyShaderModule(device, vertShader, nullptr);
}

//创建shader模块
VkShaderModule VulkanBase::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	//需传入字节编码数据的指针（类型uint32_t*），使用C++的reinterpret_cast来转换类型
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("创建shader 模块失败");
	}

	return shaderModule;
}

//创建管线布局描述符
void VulkanBase::createDescriptorSetLayout()
{
	//uniform描述符
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	//定义描述符类型，这里定义用于向shader传递uniform buffer.
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//描述符数量
	uboLayoutBinding.descriptorCount = 1;
	//shader阶段为顶点着色器
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	//图像采样相关描述符
	uboLayoutBinding.pImmutableSamplers = nullptr;//可选

	//采样器描述符绑定
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	//定义描述符类型：组合图像采样器
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;//可选
	//shader阶段为片元着色器（顶点着色器中也可以读取纹理，比如通过纹理向顶点着色器中传入数据，但大多数是在片元着色器中才有纹理的需求）
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("创建管线描述符失败");
	}
}

#pragma endregion


#pragma region 帧缓冲与命令缓冲



//创建帧缓冲区
void VulkanBase::createFramebuffers()
{
	swapChainFrameBuffers.resize(swapChainImageViews.size());
	for (auto i = 0ULL; i < swapChainImageViews.size(); ++i)
	{
		VkImageView attachments[]{ swapChainImageViews[i] };


		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("创建帧缓冲失败");
		}
	}
}

//创建命令池
void VulkanBase::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//命令缓冲所属的队列
	commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.has_value() ? queueFamilyIndices.graphicsFamily.value() : -1;
	commandPoolInfo.flags = 0;			//可选

	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("创建命令缓冲池失败");
	}
}


//建立命令缓冲
void VulkanBase::createCommandBuffer()
{
	commandBuffers.resize(swapChainFrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	//level用于指定分配的指令缓冲对象是主要指令缓冲对象还是辅助指令缓冲对象
	//VK_COMMAND_BUFFER_LEVEL_PRIMARY：主执行指令，可以被提交到队列进行执行，但不能被其它指令缓冲对象调用
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY：辅助指令，不能直接被提交到队列进行执行，但可以被主要指令缓冲对象调用执行。
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data() )!= VK_SUCCESS)
	{
		throw std::runtime_error("创建命令缓冲失败");
	}

	//记录命令到命令缓冲
	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//怎样使用命令缓冲
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：仅提交一次
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：只在一个渲染流程内使用的辅助命令缓冲。
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT：在命令缓冲等待执行时，同时可以提交这一命令缓冲。
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	//这使得我们可以在上一帧还未结束渲染时，提交下一帧的渲染指令
		//beginInfo.pInheritanceInfo = nullptr;							//可选

		//开始录制命令
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("开始录制命令缓冲失败");
		}
		{
			//设置开始渲染pass
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFrameBuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			//使用完全不透明的黑色作为清除值
			VkClearValue clearColor = { 0.1f, 0.2f, 0.5f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
			//VK_SUBPASS_CONTENTS_INLINE：所有要执行的指令都在主要指令缓冲中，没有辅助指令缓冲需要执行
			//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：有来自辅助指令缓冲的指令需要执行
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			{
				//绑定图形管线
				//第二个参数于指定管线对象是图形管线还是计算管线,VK_PIPELINE_BIND_POINT_GRAPHICS表示是图形管线
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

				//绑定顶点缓冲
				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

				//绑定索引缓冲
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

				//绑定描述符集				
				vkCmdBindDescriptorSets(commandBuffers[i],
					VK_PIPELINE_BIND_POINT_GRAPHICS, //描述符集并不是图形管线所独有的（和图形管线一样），需要指定要绑定的是图形管线还是计算管线
					pipelineLayout, 
					0,									//需要绑定的描述符集的第一个元素索引
					1,									//需要绑定的描述符集个数
					&descriptorSets[i],					//描述符集数组
					0,									//指定动态描述符的数组偏移
					nullptr
				);

				//DrawCall命令
				//vertexCount：顶点数
				//instanceCount：用于实例渲染，为1 时表示不进行实例渲染。
				//firstVertex：用于定义着色器变量gl_VertexIndex 的值
				//firstInstance：用于定义着色器变量gl_InstanceIndex 的值
				//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
				//绘制带索引的mesh
				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			}

			//结束渲染流程
			vkCmdEndRenderPass(commandBuffers[i]);
		}
		//结束录制命令
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("录制命令缓冲失败");
		}
	}


	
}

#pragma endregion

#pragma region 渲染和显示
//建立信号量和光栅对象,用于异步操作中的同步信号
//分别建立图形获取和渲染结束的信号量，一个用于通知开始渲染，另一个用于通知开始显示
void VulkanBase::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//设置初始状态为已发出信号的状态，否则vkWaitForFences将不会收到信号，会永远等待下去
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{

		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("建立信号量失败");
		}
	}

	
}

//创建管线布局的unifrom数据
void VulkanBase::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		createBuffer(bufferSize, 
			//定义用途：Uniform buffer
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT（从cpu写入数据）和 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT （保证显存可见的一致性，保证在各gpu内核中的数据一至）
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

//创建描述符池
void VulkanBase::createDescriptorPool()
{
	//描述符池size定义
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//描述符池size的个数
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	////描述符池size对象
	poolInfo.pPoolSizes = poolSizes.data();
	//最大的描述符集个数
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
	//是否可以被清除掉
	//poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("创建描述符池失败");
	}


}

//创建描述符集并写入数据
void VulkanBase::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	//描述符布局对象个数要匹配描述符集对象个数
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	//创建描述符集，描述符集会在描述符池Destroy时自动清除，所以不用显式的调用Destroy
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("创建描述符集失败");
	}

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		//配置描述符引用的缓冲对象
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		//缓存区域，如果使用VK_WHOLE_SIZE则表示使用整个缓冲
		bufferInfo.range = sizeof(UniformBufferObject);

		//绑定图像和采样器到描述符集
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		//写入数据到描述符集
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//要写入的描述符集对象
		descriptorWrites[0].dstSet = descriptorSets[i];
		//缓存绑定索引，因为dstSet不是精数组，所以下标0表示第一个描述符集对象
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		//描述符类型：uniform缓冲
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//描述符集数量
		descriptorWrites[0].descriptorCount = 1;
		//描述符集需要写入缓存对象
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		//描述符集需要写入的图像数据
		descriptorWrites[0].pImageInfo = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		//描述符类型：图像采样器
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		//执行写入
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

//绘制，每帧调用
void VulkanBase::drawFrame()
{
	//等待一组光栅(fence) 中的一个或全部光栅(fence) 发出信号,waitall设置为VK_TRUE在此处无意义，因为只有一个光栅
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	//收到光栅信号后，重置光栅对象为未发出信号的状态
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	//从交换链获取图片,KHR后缀的都是与交换链相关的操作
	uint32_t imageIndex;
	//此操作是异步的，第4、5参数为获取图象完成后通知的对象，这里通知当前帧图片信号量和当前帧的光栅对象，此处可以不用传入光栅对象
	//最后一个参数是读取的图片索引，为交换链图片数组的索引号
	//返回值表示交换链是否可用
	auto result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], inFlightFences[currentFrame], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//更新uniform数据
	updateUniformBuffer(imageIndex);

	//提交图片到指令缓冲
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//要等待的信号量对象
	VkSemaphore waitSemaphore[] = {imageAvailableSemaphores[currentFrame]};
	//需要等待的管线阶段,这里我们需要写入颜色数据到图像，所以需要等待图像管线到达可以写入颜色附着的管线阶段
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	//waitStages 数组中的条目和pWaitSemaphores中相同索引的信号量相对应。
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	//提交执行的指令缓冲对象
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	//缓冲指令结束后通知的信号量
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	// 提交到图形列队，最后一个参数为光栅对象(可选)，用于指令缓冲结束后的光栅化操作
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	
	//显示
	VkPresentInfoKHR presendInfo = {};
	presendInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presendInfo.waitSemaphoreCount = 1;
	presendInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapChain};
	presendInfo.swapchainCount = 1;
	presendInfo.pSwapchains = swapChains;
	presendInfo.pImageIndices = &imageIndex;
	presendInfo.pResults = nullptr;		//可选，每个交换链的呈现操作是否成功的信息，由于我们只使用了一个交换链，呈现函数的返回值来判断呈现操作是否成功，没有必要使用pResults
	//显示
	result = vkQueuePresentKHR(presentQueue, &presendInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	//更新当前帧,帧数在0-MAX_FRAMES_IN_FLIGHT中循环
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//更新uniform数据
void VulkanBase::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//模型的渲染设计成绕Z 轴渲染time 弧度
	UniformBufferObject ubo = {};
	//glm::rotate 函数以矩阵，旋转角度和旋转轴作为参数,glm::mat4(1.0f)为构造的单位矩阵， time * glm::radians(90.0f) 完成每秒旋转90 度的操作。
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::lookAt 函数以观察者位置，视点坐标和向上向量为参数生成视图变换矩阵
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::perspective 函数以视域的垂直角度，视域的宽高比以及近平面和远平面距离为参数生成透视变换矩阵。
	//需要注意在窗口大小改变后应该使用当前交换链范围来重新计算宽高比
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	//GLM 库最初是为OpenGL 设计的，它的裁剪坐标的Y 轴和Vulkan是相反的。将投影矩阵的Y 轴缩放系数符号取反来使投影矩阵和Vulkan 的要求一致。
	ubo.proj[1][1] *= -1;

	//复制数据到当前帧对应的uniform 缓冲
	void* data;
	vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	{
		memcpy(data, &ubo, sizeof(ubo));
	}
	vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

#pragma endregion

#pragma region 图片数据与缓存
//创建纹理视图
void VulkanBase::createTextureImageView() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM);
}

//创建采样器，采样品并不绑定VkImage，而是可以用于访问任何VkImage对象
void VulkanBase::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//指定纹理需要放大和缩小时的插值，纹理放大会出现采样过密的问题，纹理缩小会出现采样过疏的问题
	//VK_FILTER_NEAREST或VK_FILTER_LINEAR分别对应两种情况
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	//寻址模式，uvw分别对应xyz
	//VK_SAMPLER_ADDRESS_MODE_REPEAT：采样超出图像范围时重复纹理
	//VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT：采样超出图像范围时重复镜像后的纹理
	//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE：采样超出图像范围时使用距离最近的边界纹素
	//VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE：采样超出图像范围时使用镜像后距离最近的边界纹素
	//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER：采样超出图像返回时返回设置的边界颜色
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	//开启各向异性过滤
	samplerInfo.anisotropyEnable = VK_TRUE;
	//各向异性最大样本个数，一般最大为16，数量与性能成反比，与品质成正比
	samplerInfo.maxAnisotropy = 16;
	//用于VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER寻址模式时的边界颜色
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	//坐标系统，true时为0-width和0-height，为false时为0-1，一般为false,与shader中的一至
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	//启用比较（可用于阴影等）
	samplerInfo.compareEnable = VK_FALSE;
	//比较后的操作（可用于阴影等）
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	//mipmapMode、mipLodBias、minLod 和maxLod 成员变量用于设置分级细化(mipmap)
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("创建纹理采样器失败");
	}
}

//创建纹理缓冲
void VulkanBase::createTextureImage()
{
	//读取到的图片尺寸和实际存在的颜色通道
	int texWidth, texHeight, texChannels;
	//STBI_rgb_alpha为rgba四通道，每个像素4个字节，总尺寸为width * height * 4字节
	stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels)
	{
		throw std::runtime_error("读取图片textures/texture.jpg失败");
	}

	//临时缓冲区（cpu 可见区域）
	VkBuffer tempBuffer;
	VkDeviceMemory tempBufferMemory;
	createBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,	//作为传输来源
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //从CPU 写入数据 和 保持各GPU内核的一至性
		tempBuffer, tempBufferMemory);

	//复制图片到临时缓冲区
	void *data;
	vkMapMemory(device, tempBufferMemory, 0, imageSize, 0, &data);
	{
		memcpy(data, pixels, static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(device, tempBufferMemory);
	//清理加载的图片（cpu内存已经不需要了）
	stbi_image_free(pixels);

	//创建正式缓存
	createImage(texWidth, texHeight, 
		VK_FORMAT_R8G8B8A8_UNORM, //STBI_rgb_alpha类型对应的格式
		//tiling值 ：VK_IMAGE_TILING_LINEAR：纹素以行主序的方式排列，如果要直接访问图片数据，则需以设置为此
		//tiling值 ：K_IMAGE_TILING_OPTIMAL：纹素以一种对访问优化的方式排列,临时缓冲等不需要处理的图片设置为此更好性能
		VK_IMAGE_TILING_OPTIMAL,
		//传输目标 和 被shader采样
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		textureImage, textureImageMemory);

	//转换图像布局，
	//第一次转换，目标：允许读取图像数据作为传输目的，这样才能执行复制
	transitionImageLayout(textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_UNDEFINED,	//old layout，VK_IMAGE_LAYOUT_UNDEFINED表示不需要访问之前的数据，必能更好
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL	//new layout ，传输目的
	);
	//执行复制，从临时缓冲到正式缓冲
	copyBufferToImage(tempBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	//第二次转换，目标：允许shader读取，这样在片元着色器中才能对图片进行采样
	transitionImageLayout(textureImage, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		//old layout，传输目的
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	//new layout shader中读取
	);

	vkDestroyBuffer(device, tempBuffer, nullptr);
	vkFreeMemory(device, tempBufferMemory, nullptr);

}
//工具：创建图像缓冲区
void VulkanBase::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("创建图形缓存失败");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("创建图形缓存的内存区域失败");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

//工具：变换图像布局
void VulkanBase::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	//对图像布局进行变换
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	//来源队列族索引，VK_QUEUE_FAMILY_IGNORED表示忽略队列族所有权
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//目标队列族索引，VK_QUEUE_FAMILY_IGNORED表示忽略队列族所有权
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//进行布局变换的图像对图像对象及其属性
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	//发生在屏障之前的管线阶段
	VkPipelineStageFlags sourceStage;
	//发生在屏障之后的管线阶段
	VkPipelineStageFlags destinationStage;	
	
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		//第一次变换时，即从未定义到传输目的
		//来源：未定义
		barrier.srcAccessMask = 0;
		//目的：写入
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//上个阶段：初始管线阶段（虚拟）
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		//目标阶段：传输阶段（虚拟）
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		//第二次变换时，即从传输目的到shader读取
		//来源：写入
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//目的：shader读取
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//上个阶段：传输阶段（虚拟）
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		//目标阶段：片元着色器（真实）
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	//管线屏障指令，用于同步资源访问，可以用于保证图像在被读取之前数据被写入也可以被用来变换图像布，也可以被用来变换图像布局
	//变换图像布局的指令
	vkCmdPipelineBarrier(
		commandBuffer,		
		sourceStage,		//发生在屏障之前的管线阶段
		destinationStage,	//发生在屏障之后的管线阶段
		0,					//dependencyFlags,设置为VK_DEPENDENCY_BY_REGION_BIT 的话，屏障就变成了一个区域条件
		0,					//memoryBarrierCount	内存屏
		nullptr,			//pMemoryBarriers
		0,					//bufferMemoryBarrierCount 缓冲内存屏障
		nullptr,			//pBufferMemoryBarriers
		1,					//imageMemoryBarrierCount 图像屏障
		&barrier			//pImageMemoryBarriers
	);

	endSingleTimeCommands(commandBuffer);
}

//复制缓冲到图像
void VulkanBase::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	//以下两个为数据在内存中的存放方式，设置为0为紧凑排列
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer,
		buffer, 
		image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, 
		&region
	);

	endSingleTimeCommands(commandBuffer);
}
//工具：开始录制单次命令
VkCommandBuffer VulkanBase::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//类型为主执行指令
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//仅提交一次,命令只需执行一次
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//开始录制
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}
//工具，结束录制单次命令
void VulkanBase::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	//提交命令缓冲到图形队列
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//光栅对象对空，无需异步等待
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	//等待操作完成，与光栅对象fence不一样，由于仅需执行一次，这里用同步操作就行了
	vkQueueWaitIdle(graphicsQueue);

	//销毁命令缓冲对象
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//工具：创建纹理视图
VkImageView VulkanBase::createImageView(VkImage image, VkFormat format) {

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	//指定图像的用途和图像的哪一部分可以被访问。在这里，我们的图像被用作渲染目标，并且没有细分级别，只存在一个图层,VR程序可能会有多个图图层。
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("创建纹理视图失败!");
	}

	return imageView;
}

#pragma endregion

#pragma region 顶点数据与GPU缓存
//读取网格模型、建立顶点数据
void VulkanBase::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();;

	//临时缓存(gpu)
	//临时缓存将采用CPU可以访问的缓存类型，将cpu 内存复制到此缓存中
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize,		
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,	//传输来源
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,////需要位域满足VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT（从cpu写入数据）和 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT （保证显存可见的一致性，保证在各gpu内核中的数据一至）
		stagingBuffer,
		stagingBufferMemory
	);

	void *data;
	//内存<->显存映射,将VKDeviceMemory映射到CPU内存
	//offet和size用来指定的内存偏移量和大小，还有一个特殊值VK_WHOLE_SIZE 可以用来映射整个申请的内存
	//flags为预留的标记，暂未用到，必须填0
	//data为内存映射后的地址
	//虽然在代码上完成了内存到显存的复制，但所有gpu内核中不一定立即可见，也不一定同时可见，因为现代处理器都有缓存这一机制。前面选择显存类型时要求有VK_MEMORY_PROPERTY_HOST_COHERENT_BIT正是为了避免各内核数据不一至。
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	{
		//将cpu内存数据复制到映射地址中
		memcpy(data, vertices.data(), (size_t)bufferSize);
	}
	//结束内存<->显存的映射
	vkUnmapMemory(device, stagingBufferMemory);

	//正式缓存(gpu)，CPU无法访问，但GPU访问的性能更好
	createBuffer(bufferSize,
		//指定顶点数据的使用目的，可以用位运算来指定多个。
		//VK_BUFFER_USAGE_TRANSFER_SRC_BIT 用于内存传输操作的数据来源
		//VK_BUFFER_USAGE_TRANSFER_DST_BIT 用于内存传输操作的数据目标
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,//传输目的与顶点数
		//类型为设备内部内存，CPU无法访问，但GPU访问的性能更好
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	//复制临时缓存到正式缓存（通过GPU的命令缓冲）
	copyBUffer(stagingBuffer, vertexBuffer, bufferSize);

	//清除临时缓冲,copyBUffer是同步执行的，不是采用光栅对象或信号量的异步操作，到这里时已经完成了复制命令的提交了
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

}

//索引数据
void VulkanBase::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	//临时缓冲
	VkBuffer tempBuffer;
	VkDeviceMemory tempBufferMemory;
	createBuffer(bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,//使用目的：传输数据源
		//需要位域满足VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT（从cpu写入数据）和 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT （保证显存可见的一致性，保证在各gpu内核中的数据一至）
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		tempBuffer, tempBufferMemory
	);

	//CPU到临时缓冲的内存映射
	void *data;
	vkMapMemory(device, tempBufferMemory, 0, bufferSize, 0, &data);
	{
		memcpy(data, indices.data(), (size_t)bufferSize);
	}
	vkUnmapMemory(device, tempBufferMemory);

	//创建正式缓冲
	createBuffer(bufferSize,
		//使用目的，传输目标与索引数据
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,indexBufferMemory
	);

	copyBUffer(tempBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, tempBuffer, nullptr);
	vkFreeMemory(device, tempBufferMemory, nullptr);
}

//工具：建立缓冲
void VulkanBase::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	//指定顶点数据的使用目的，可以用位运算来指定多个。
	bufferInfo.usage = usage;
	//指定队列族的共享模式，这里只有一个队列，所以为独占模式。
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	//返回的数据
	//需要的内存尺寸
	//memRequirements.size
	//内存区域开始位置
	//memRequirements.alignment
	//指示适合的内存类型的位域
	//memRequirements.memoryTypeBits
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	//分配内存
	//todo::显卡对分配内存的调用次数是有限制的，一般做法是一次性创建一个较大的内存，然后在应用程序中再自行分配给每个对象。也可使用GPUOpen的VulkanMemoryAllocator内存分配器
	//本demo中暂时使用vkAllocateMemory，是不能用于实际项目中的
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;	
	//查找物理设备上支持的显存类型
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	//绑定显存区域与缓冲对象
	//第四个参数的偏移值，需要能被memRequirements.alignment整除
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
//工具：复制缓存数据，通过命令缓冲
void VulkanBase::copyBUffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);

	////创建命令缓冲
	//VkCommandBufferAllocateInfo allocInfo = {};
	//allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//类型为主执行指令
	//allocInfo.commandPool = commandPool;
	//allocInfo.commandBufferCount = 1;
	//VkCommandBuffer commandBuffer;
	//vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
	////录制命令
	//VkCommandBufferBeginInfo beginInfo = {};
	//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	////仅提交一次,命令只需执行一次
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//vkBeginCommandBuffer(commandBuffer, &beginInfo);
	//{
	//	VkBufferCopy copyRegion = {};
	//	copyRegion.srcOffset = 0;
	//	copyRegion.dstOffset = 0;
	//	copyRegion.size = size;
	//	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	//}
	//vkEndCommandBuffer(commandBuffer);
	////提交命令缓冲到图形队列
	//VkSubmitInfo submitInfo = {};
	//submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &commandBuffer;
	////光栅对象对空，无需异步等待
	//vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	////等待操作完成，与光栅对象fence不一样，由于仅需执行一次，这里用同步操作就行了
	//vkQueueWaitIdle(graphicsQueue);
	////销毁命令缓冲对象
	//vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//工具：选择合适的显存类型
uint32_t VulkanBase::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	//遍历数组，查找缓冲可用的内存类型
	//相应位域为1	
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("未找到合适的显存类型");
}
#pragma endregion



