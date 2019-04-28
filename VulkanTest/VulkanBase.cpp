
#include "VulkanBase.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int WIDTH = 1280;
const int HEIGHT = 720;

//校验层
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};




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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanBase::initVulkan() {
	createInstance();		//创建实例
	setupDebugMessenger();	//设置校验层
	createSurface();		//创建窗口表面
	pickPhysicalDevice();	//选择物理设备和队列族
	createLogicalDevice();	//逻辑设备和队列
	createSwapChain();		//创建交换链
	createImageViews();		//创建图像视图
	createRenderPass();		//创建渲染pass
	createGraphicsPipeline();	//创建图形管线
	createFramebuffers();		//创建帧缓冲
	createCommandPool();		//创建创建命令池
	createCommandBuffer();		//建立命令缓冲
	createSemaphores();			//建立信号量
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

	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandBuffers.clear();
	vkDestroyPipeline(device, graphicsPipeline,nullptr);	
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);	
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto frameBuffer : swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(device,frameBuffer,nullptr);
	}
	swapChainFrameBuffers.clear();

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	swapChainImageViews.clear();

	vkDestroySwapchainKHR(device, swapChain, nullptr);
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
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
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

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
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

	//调用glf来完成窗体表面创建，避免仅支持windows平台
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

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//VK_SHARING_MODE_EXCLUSIVE：一张图像同一时间只间只能被一个队列族所拥有，在另一队列族使用它之前，必须显式地改变图像所有权。这一模式下性能表现最佳
	//VK_SHARING_MODE_CONCURRENT：图像可以在多个队列族间使用，不需要显式地改变图像所有权。	
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

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//指定alpha 通道是否被用来和窗口系统中的其它窗口进行混合操作,这里将其设置为VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR来忽略掉alpha 通道。
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
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

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
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
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

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

#pragma endregion

#pragma region 图像视图
void VulkanBase::createImageViews() {
	//分配足够的数组空间来存储图像视图
	swapChainImageViews.resize(swapChainImages.size());

	//遍历所有交换链图像，创建图像视图
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		//指定图像被看作是一维纹理、二维纹理、三维纹理还是立方体贴图
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		//进行图像颜色通道的映射,对于单色纹理，我们可以将所有颜色通道映射到红色通道我们也可以直接将颜色	通道的值映射为常数0 或1,在这里，我们只使用默认的映射
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//指定图像的用途和图像的哪一部分可以被访问。在这里，我们的图像被用作渲染目标，并且没有细分级别，只存在一个图层,VR程序可能会有多个图图层。
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		//调用vkCreateImageView 函数创建图像视图
		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
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
	//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL：图像被用作复制操作的目的图像
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


//创建图形管线
void VulkanBase::createGraphicsPipeline() {

	//读取shader字节（需读取编译后的shader，先使用glslangValidator.exe编译shader代码）
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");
	//创建shader模块（字节封装）
	auto vertShader = createShaderModule(vertShaderCode);
	auto fragShader = createShaderModule(fragShaderCode);

	//创建shader阶段，这里确定了shader的类型(frag或vectex)、执行函数
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;//顶点着色器
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName = "main";
	//以下代码可以向shader传递define，这些define一般用于shader内部的分支#if #else #endif
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;//片元着色器
	fragShaderStageInfo.module = fragShader;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo ,fragShaderStageInfo };


	//顶点输入
	//todo::读取网格(mesh)文件传入
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	//输入装配
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//几何图元的类型
	//VK_PRIMITIVE_TOPOLOGY_POINT_LIST：点图元
	//VK_PRIMITIVE_TOPOLOGY_LINE_LIST：每两个顶点构成一个线段图元
	//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP：每两个顶点构成一个线段图元，除第一个线段图元外，每个线段图元使用上一个线段图元的一个顶点
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST：每三个顶点构成一个三角形图元
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP：每个三角形的第二个和第三个顶点被下一个三角形作为第一和第二个顶点使用
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//是否启用几何图元重启
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//视口和裁剪
	//视口
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

	//光栅化
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//epthClampEnable 设置为VK_TRUE 表示在近平面和远平面外的片段会被截断为在近平面和远平面上，而不是直接丢弃这些片段。这对于阴影贴图的生成很有用
	rasterizer.depthClampEnable = VK_FALSE;
	//rasterizerDiscardEnable 设置为VK_TRUE 表示所有几何图元都不能通过光栅化阶段。这一设置会禁止一切片段输出到帧缓冲。
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
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//可选
	rasterizer.depthBiasClamp = 0.0f;			//可选
	rasterizer.depthBiasSlopeFactor = 0.0f;		//可选


	//多重采样
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;				//可选
	multisampling.pSampleMask = nullptr;				//可选
	multisampling.alphaToCoverageEnable = VK_FALSE;		//可选
	multisampling.alphaToOneEnable = VK_FALSE;			//可选

	//深度和模板测试
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	

	//颜色混合
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.blendEnable = VK_TRUE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
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


	//管线部局
	//管理部局可以用于设置shader中的uniform变量	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;						//可选
	pipelineLayoutInfo.pSetLayouts = nullptr;					//可选
	pipelineLayoutInfo.pushConstantRangeCount = 0;				//可选
	pipelineLayoutInfo.pPushConstantRanges = nullptr;			//可选

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("创建管线部局失败");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;				//可选
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;					//可选

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
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
	//VK_COMMAND_BUFFER_LEVEL_PRIMARY：可以被提交到队列进行执行，但不能被其它指令缓冲对象调用
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY：不能直接被提交到队列进行执行，但可以被主要指令缓冲对象调用执行。
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
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：命令缓冲在执行一次后，就被用来记录新的命令
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：这是一个只在一个渲染流程内使用的辅助命令缓冲。
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT：在命令缓冲等待执行时，仍然可以提交这一命令缓冲。
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	//这使得我们可以在上一帧还未结束渲染时，提交下一帧的渲染指令
		beginInfo.pInheritanceInfo = nullptr;							//可选

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("开始录制命令缓冲失败");
		}

		//设置开始渲染pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFrameBuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		//使用完全不透明的黑色作为清除值
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		//VK_SUBPASS_CONTENTS_INLINE：所有要执行的指令都在主要指令缓冲中，没有辅助指令缓冲需要执行
		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：有来自辅助指令缓冲的指令需要执行
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//绑定图形管线
		//第二个参数于指定管线对象是图形管线还是计算管线,VK_PIPELINE_BIND_POINT_GRAPHICS表示是图形管线
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		//调用指令进行三角形的绘制操作
		//vertexCount：尽管这里我们没有使用顶点缓冲，但仍然需要指定三个顶点用于三角形的绘制
		//instanceCount：用于实例渲染，为1 时表示不进行实例渲染。
		//firstVertex：用于定义着色器变量gl_VertexIndex 的值
		//firstInstance：用于定义着色器变量gl_InstanceIndex 的值
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		//结束渲染流程
		vkCmdEndRenderPass(commandBuffers[i]);

		//结束记录指令
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("录制命令缓冲失败");
		}
	}


	
}

#pragma endregion

#pragma region 渲染和显示
//建立信号量,用于异步操作中的同步信号
//分别建立图形获取和渲染结束的信号量，一个用于通知开始渲染，另一个用于通知开始显示
void VulkanBase::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("建立信号量失败");
	}

	
}

//绘制，每帧调用
void VulkanBase::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(device, 1, &inFlightFence);

	//从交换链获取图片,KHR后缀的都是与交换链相关的操作
	uint32_t imageIndex;
	//此操作是异步的，第4、5参数为获取图象完成后通知的对象，这里通知信号量
	//最后一个参数是读取的图片索引，为交换链图片数组的索引号
	vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	//提交图片到指令缓冲
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//要等待的信号量对象
	VkSemaphore waitSemaphore[] = {imageAvailableSemaphore};
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
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	// 提交到图形列队，最后一个参数为光栅对象(可选)，用于指令缓冲结束后的光栅化操作，这里我们需要等待信号通知后自定义操作，所以不用传入光栅对象	
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
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
	vkQueuePresentKHR(presentQueue, &presendInfo);


}


#pragma endregion



