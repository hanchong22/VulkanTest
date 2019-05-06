
#include "VulkanBase.h"

//stb��ͷ�ļ��ж�û�м�#pragma once������ֻ����cpp���ã�����Ϳ��ܳ����ظ����õ�LNK2005����
#define STB_IMAGE_IMPLEMENTATION	//����std���е�ͼƬ����
#include <stb_image.h>				//��ȡͼƬ����	


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif




//���ڴ�С�ı�ʱ�Ļص�
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}


#pragma region ��������

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
	//�����ڴ�С�ı�
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	//�ѵ�ǰ��������Ϊ�û��Զ�������Ա��ڻص��������ҵ���ǰVulkanBaseʵ��
	glfwSetWindowUserPointer(window, this);
	//���ڴ�С�ı�ʱ�Ļص�
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}



void VulkanBase::initVulkan() {
	createInstance();		//����ʵ��
	setupDebugMessenger();	//����У���
	createSurface();		//�������ڱ���
	pickPhysicalDevice();	//ѡ�������豸�Ͷ�����
	createLogicalDevice();	//�߼��豸�Ͷ���
	createSwapChain();		//����������
	createImageViews();		//����������ͼ����ͼ
	createRenderPass();		//������Ⱦpass
	createDescriptorSetLayout();	//�������߲���������
	createGraphicsPipeline();	//����ͼ�ι���
	createFramebuffers();		//����֡����
	createCommandPool();		//�������������
	createTextureImage();		//��������
	createTextureImageView();	//����������ͼ
	createTextureSampler();		//����������
	createVertexBuffer();		//�����������ݻ���
	createIndexBuffer();		//�����������ݻ���
	createUniformBuffers();		//����unifrom����
	createDescriptorPool();		//������������
	createDescriptorSets();		//���������Ἧ
	createCommandBuffer();		//���������
	createSyncObjects();		//����ͬ������
}

//��ѭ��
void VulkanBase::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}


void VulkanBase::cleanup() {	

	cleanupSwapChain();

	//���㻺�岻����������
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


#pragma region ʵ��������У�������

//����ʵ��
void VulkanBase::createInstance() {

	//���У���
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("û���ҵ�У���!");
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

	//У�����չ
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//У���
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



//����У�����Ϣ����
void VulkanBase::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	//messageSeverity���ûص�����
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//messageType��Ϣ����
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	//�ص�����
	createInfo.pfnUserCallback = debugCallback;
	//��ѡ��ص�ʱ�Ĳ���
	createInfo.pUserData = nullptr;

	//����vkCreateDebugUtilsMessengerEXT
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}



//�����Ƿ�����У��㣬�����������չ�б�
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

//������п��õ�У���
bool VulkanBase::checkValidationLayerSupport() {

	//���п��õ�У�����
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	//������е�У����Ƿ����ҵ�
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

//vkCreateDebugUtilsMessengerEXT�����Ĵ�����
//����vkCreateDebugUtilsMessengerEXT ������һ����չ����,���ᱻVulkan ���Զ����أ�������Ҫ�����Լ�ʹ��vkGetInstanceProcAddr ������������
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


#pragma region �����豸�Ͷ��������߼�����
//ѡ�������豸�Ͷ�����
void VulkanBase::pickPhysicalDevice() {
	//��һ�ε��ã���������豸�������Ա��ʼ������������
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	//�õ���������еڶ��ε��ã����ؿ��õ������豸
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//ѡ���һ��������������Կ�
	//todo:: ��ÿ���豸��֣�ѡ�������ߵ��豸
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

//���������豸�Ƿ�����������
bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	//��ȡ�����豸֧�ֵ�������Ϣ
	//��Ҫ֧�ֲ����������Թ���
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



//�����߼��豸�����
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
	//���������������֧��
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
	//��ȡָ��������Ķ��о��
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

//������������Ķ�����
QueueFamilyIndices VulkanBase::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	//���Ȼ�ȡ�ж��������Ȼ����ݸ�������������ٲ��ҳ������ж���
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	//���������ж��壬���֧��VK_QUEUE_GRAPHICS_BIT��Physical���ж���
	//�ҵ���һ���������ߵ��ж���
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

		//��������
		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}
#pragma endregion

#pragma region ���ڱ���
//�������ڱ���
void VulkanBase::createSurface() {

	//����glfw�����VKsurface����������Ҫglfw��֧�֣�3.0�汾��glfw�Ѿ�֧��vulkan
	//�˲�����������vulkanʵ����ϵ����
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}	
}
#pragma endregion

#pragma region ������

//����������
void VulkanBase::createSwapChain() {
	//������ϸ����Ϣ
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	//ѡ������ʽ
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//����ģʽ
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//������Χ
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//ʹ�ý�����֧�ֵ���Сͼ����� + 1 ������ͼ����ʵ����������
	//maxImageCount ��ֵΪ0 ������ֻҪ�ڴ�������㣬���ǿ���ʹ������������ͼ��
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
	//ÿ��ͼ���������Ĳ�Σ�һ��Ϊ1��VR�����
	createInfo.imageArrayLayers = 1;
	//��ͼ���ϵĲ���,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT��ʾͼ�����Ϊ����Ŀ��ͼ��
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//ָ�����������ʹ�ý�����ͼ��ķ�ʽ,����ͼ�ζ��кͳ��ֶ��в���ͬһ�����е�������źܴ�Ӱ��,ͨ��ͼ�ζ����ڽ�����ͼ���Ͻ��л��Ʋ�����Ȼ��ͼ���ύ�����ֶ�������ʾ
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//VK_SHARING_MODE_EXCLUSIVE�� ��ռģʽ��һ��ͼ��ͬһʱ��ֻ��ֻ�ܱ�һ����������ӵ�У�����һ������ʹ����֮ǰ��������ʽ�ظı�ͼ������Ȩ����һģʽ�����ܱ������
	//VK_SHARING_MODE_CONCURRENT��Эͬģʽ��ͼ������ڶ���������ʹ�ã�����Ҫ��ʽ�ظı�ͼ������Ȩ��	
	if (indices.graphicsFamily != indices.presentFamily) {
		//���ͼ�κͳ��ֲ���ͬһ�������壨���������������ʹ��Эͬģʽ�����⴦��ͼ������Ȩ���⡣Эͬģʽ��Ҫ����ʹ��queueFamilyIndexCount ��pQueueFamilyIndices ��ָ����������Ȩ�Ķ�����
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		//���ͼ�ζ�����ͳ��ֶ�������ͬһ��������(�󲿷�����¶�������)�����ǾͲ���ʹ��Эͬģʽ��Эͬģʽ��Ҫ����ָ������������ͬ�Ķ����塣
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	//Ϊ�������е�ͼ��ָ��һ���̶���Transfrom�仯����
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//ָ��alpha ͨ���Ƿ������ʹ���ϵͳ�е��������ڽ��л�ϲ���,���ｫ������ΪVK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR�����Ե�alpha ͨ����
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//ָ��ǰ��ѡ�õ���ʾģʽ
	createInfo.presentMode = presentMode;
	//clipped ��Ա����������ΪVK_TRUE ��ʾ���ǲ����ı�����ϵͳ�е����������ڵ������ص���ɫ��������Vulkan ��ȡһ�����Ż���ʩ����������ǻض����ڵ�����ֵ�Ϳ��ܳ�������
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

//���������
void VulkanBase::cleanupSwapChain()
{
	for (auto frameBuffer : swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}
	swapChainFrameBuffers.clear();

	//commandBuffers����Ҫ�ؽ���ֻ�����vkFreeCommandBuffers��������ָ������
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

//ѡ�񽻻��������ʽ
//ѡ��֧��RGB��SRGB��ʽ
VkSurfaceFormatKHR VulkanBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//����ѡ��SRGB
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	//Ĭ�Ϸ��ص�1��
	return availableFormats[0];
}

//ѡ�����ģʽ
VkPresentModeKHR VulkanBase::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

	//����ģʽ��
	//VK_PRESENT_MODE_IMMEDIATE_KHR��Ӧ�ó����ύ��ͼ	��ᱻ�������䵽��Ļ�ϣ����ܻᵼ��˺������
	//VK_PRESENT_MODE_FIFO_KHR�����������һ���Ƚ��ȳ��Ķ��У�ÿ�δӶ���ͷ��ȡ��һ��ͼ�������ʾ��
	//--------Ӧ�ó�����Ⱦ��ͼ���ύ���������󣬻ᱻ���ڶ���β����������Ϊ��ʱ��Ӧ�ó�����Ҫ���еȴ�����һģʽ�ǳ��������ڳ��õĴ�ֱͬ����ˢ����ʾ��ʱ��Ҳ��������ֱ��ɨ��
	//VK_PRESENT_MODE_FIFO_RELAXED_KHR����һģʽ���ϸ�ģʽ��Ψһ�����ǣ����Ӧ�ó����ӳ٣����½������Ķ�������һ�δ�ֱ��ɨʱΪ�գ���ô�����Ӧ�ó�������һ�δ�ֱ��ɨǰ�ύͼ��ͼ�����������ʾ����һģʽ���ܻᵼ��˺������
	//VK_PRESENT_MODE_MAILBOX_KHR����һģʽ�ǵڶ���ģ����һ�����֡��������ڽ������Ķ�����ʱ����Ӧ�ó��򣬶����е�ͼ��ᱻֱ���滻ΪӦ�ó������ύ��ͼ����һģʽ��������ʵ���������壬����˺�������ͬʱ��С���ӳ����⡣

	//�������ֳ���ģʽ��ֻ��VK_PRESENT_MODE_FIFO_KHR ģʽ��֤һ�����ã�������Ҫ�����ѡ��һ�����ģʽ(VK_PRESENT_MODE_MAILBOX_KHR����)
	//����ģʽ��������ʱ��ʹ�ô�ģʽ
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;//����ģʽ
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;//����ģʽ
		}
	}

	return bestMode;
}

//ѡ�񽻻���Χ
//������Χ�ǽ�������ͼ��ķֱ��ʣ����������Ǻ�����Ҫ��ʾͼ��Ĵ��ڵķֱ�����ͬ
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

//��ѯ������ϸ����Ϣ
SwapChainSupportDetails VulkanBase::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	//��ѯ���������ڱ�������
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	//��ѯ����֧�ֵĸ�ʽ
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//��ѯ֧�ֵĳ���ģʽ
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


//����VkImageView�����ڷ��ʽ������е�ͼ��
void VulkanBase::createImageViews() {
	//�����㹻������ռ����洢ͼ����ͼ
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
	}

}
#pragma endregion

#pragma region ͼ�ι�����shader

//������Ⱦpass
void VulkanBase::createRenderPass()
{
	//���帽�������ڽ�����ͼ�����ɫ����
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//������Ⱦǰ����
	//K_ATTACHMENT_LOAD_OP_LOAD�����ָ�������������
	//VK_ATTACHMENT_LOAD_OP_CLEAR��ʹ��һ������ֵ���������������
	//VK_ATTACHMENT_LOAD_OP_DONT_CARE�������ĸ����ִ������
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//ÿ����Ⱦ�µ�һ֡ǰʹ�ú�ɫ���֡����
	//������Ⱦ�����
	//VK_ATTACHMENT_STORE_OP_STORE����Ⱦ�����ݻᱻ�洢�������Ա�֮���ȡ
	//VK_ATTACHMENT_STORE_OP_DONT_CARE����Ⱦ�󣬲����ȡ֡���������
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//����ģ�建��
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//������Ⱦǰ���ͼ���ʽ
	//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL��ͼ��������ɫ����
	//VK_IMAGE_LAYOUT_PRESENT_SRC_KHR��ͼ�����ڽ������н��г��ֲ���
	//VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL�� ������Ϊ���Ʋ�������Դ(vkCmdCopyImageToBuffer)
	//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL��ͼ���������Ʋ�����Ŀ��ͼ��(��vkCmdCopyBufferToImage)
	//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL��	��������shader�ж�ȡ
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//������֮ǰ��ͼ�񲼾ַ�ʽ
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//��Ⱦ���ͼ��ɱ�����������

	//��������
	VkAttachmentReference colorAttchmentRef = {};
	//attachmentΪ����������
	colorAttchmentRef.attachment = 0;
	//���ַ�ʽ
	colorAttchmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	

	//��Ⱦ��pass
	VkSubpassDescription subpass = {};
	//pass����Ϊͼ����Ⱦ���Ǽ��㣩
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//ָ����ɫ����
	subpass.colorAttachmentCount = 1;
	//pColorAttachments��Ӧfragment shader�е�layout(location = 0) out vec4 outColor
	//pInputAttachments������ɫ����ȡ�ĸ���
	//pResolveAttachments�����ڶ��ز�������ɫ����
	//pDepthStencilAttachment��������Ⱥ�ģ�����ݵĸ���
	//pPreserveAttachments��û�б���һ������ʹ�ã�����Ҫ�������ݵĸ���
	subpass.pColorAttachments = &colorAttchmentRef;

	//��Ⱦpass
	//��Ⱦpass createinfo
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	

	//��pass������
	//��pass��Ҫ����ͼ�Σ��������õȴ��ź�����Ҳ�������õȴ����߽׶�
	VkSubpassDependency dependency = {};
	//��Ⱦ���̿�ʼǰ����pass
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;		//�������������̵�����,VK_SUBPASS_EXTERNALΪ������������
	//��Ⱦ���̽��������pass,����0��ǰ�洴������pass��������Ϊ�˱���ѭ������,dstSupass�������srcSubpass
	dependency.dstSubpass = 0;							//�����������������̵�����
	//srcStageMask ��srcAccessMask ��Ա��������ָ����Ҫ�ȴ��Ĺ��߽׶κ������̽����еĲ������͡�
	//������Ҫ�ȴ�������������ͼ��Ķ�ȡ���ܶ�ͼ����з��ʲ�����Ҳ���ǵȴ���ɫ���������һ���߽׶Ρ�
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	//dstStageMask ��dstAccessMask ��Ա��������ָ����Ҫ�ȴ��Ĺ��߽׶κ������̽����еĲ������͡���������ǵ�����Ϊ�ȴ���ɫ���ŵ�����׶Σ�
	//�����̽��������ɫ���ŵĶ�д�������������ú�ͼ�񲼾ֱ任ֱ����Ҫʱ�Ż���У������ǿ�ʼд����ɫ����ʱ��
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("������Ⱦpassʧ��");
	}
}


//����ͼ�ι��ߡ�����shader�ֽڱ��루������shader��
void VulkanBase::createGraphicsPipeline() {

	//��ȡshader�ֽڣ����ȡ������shader����ʹ��glslangValidator.exe����shader���룩
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");
	//����shaderģ�飨�ֽڷ�װ��
	auto vertShader = createShaderModule(vertShaderCode);
	auto fragShader = createShaderModule(fragShaderCode);

	//����shader�׶Σ�����ȷ����shader������(frag��vectex)��ִ�к���
	//����vectexshader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;//������ɫ��
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName = "main";
	//���´��������shader����define����Щdefineһ������shader�ڲ��ķ�֧#if #else #endif
	//vertShaderStageInfo.pSpecializationInfo = nullptr;

	//����fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;//ƬԪ��ɫ��
	fragShaderStageInfo.module = fragShader;
	fragShaderStageInfo.pName = "main";
	//��shader���ݵ�Ԥ���붨��
	//fragShaderStageInfo.pSpecializationInfo = nullptr;

	std::vector< VkPipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo ,fragShaderStageInfo };	


	//��������
	//todo::��ȡ����(mesh)�ļ����룬��demo��û�ж�ȡmesh�ļ�������Ϊ��shader�����ж����˶������ݡ�
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//����Assembly����������������
	//Assembly����ָ���������ݶ������������͵ļ���ͼԪ���Լ��Ƿ����ü���ͼԪ�ظ�����
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//����ͼԪ������
	//VK_PRIMITIVE_TOPOLOGY_POINT_LIST����ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_LINE_LIST��ÿ�������㹹��һ���߶�ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP��ÿ�������㹹��һ���߶�ͼԪ������һ���߶�ͼԪ�⣬ÿ���߶�ͼԪʹ����һ���߶�ͼԪ��һ������
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST��ÿ�������㹹��һ��������ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP��ÿ�������εĵڶ����͵��������㱻��һ����������Ϊ��һ�͵ڶ�������ʹ��
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//�Ƿ����ü���ͼԪ�ظ����ã���ָ��������ظ������ڶ���ط����Խ�ʡmesh���ݳߴ�
	//�����Ҫ�ظ����ö������ݣ���ͨ����������ֵ 0xFFFF	��0xFFFFFFFF���������¶��嵽��������ĵ�һ���±�
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//�ӿ�Viewport�Ͳü�Rect
	//Viewport�����ڶ��������Ⱦ�����֡����������demo�л�ʹ���һ����
	VkViewport viewPort = {};
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = (float)swapChainExtent.width;
	viewPort.height = (float)swapChainExtent.height;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	//�ü�
	//��demo�к��ӿ�һ����
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = swapChainExtent;

	//�����ӿںͲü�
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewPort;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//��դ������ Rasterization	
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//depthClampEnable ����ΪVK_TRUE ��ʾ�ڽ�ƽ���Զƽ�����ƬԪ�ᱻ�ض�Ϊ�ڽ�ƽ���Զƽ���ϣ�������ֱ�Ӷ�����ЩƬԪ���������Ӱ��ͼ�����ɺ�����
	rasterizer.depthClampEnable = VK_FALSE;
	//rasterizerDiscardEnable ����ΪVK_TRUE ��ʾ���м���ͼԪ������ͨ����դ���׶Ρ���һ���û��ֹһ��ƬԪ�����֡���塣
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	//polygonMode ָ������ͼԪ����Ƭ�εķ�ʽ
		//VK_POLYGON_MODE_FILL����������Σ�����������ڲ�������Ƭ��
		//VK_POLYGON_MODE_LINE��ֻ�ж���εı߻����Ƭ��
		//VK_POLYGON_MODE_POINT��ֻ�ж���εĶ�������Ƭ��
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	//lineWidthָ����դ������߶ο�ȣ������߿���ռ��Ƭ����ĿΪ��λ���߿�����ֵ������Ӳ����ʹ�ô���1.0f ���߿���Ҫ������Ӧ��GPU ���ԡ�
	rasterizer.lineWidth = 1.0f;
	//cullModeָ��ʹ�õı����޳�����,����ͨ�������ñ����޳����޳����棬�޳����棬�Լ��޳�˫��
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//frontFace��ָ��˳ʱ��Ķ����������棬������ʱ��Ķ�����������
	//rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//��ѡ
	rasterizer.depthBiasClamp = 0.0f;			//��ѡ
	rasterizer.depthBiasSlopeFactor = 0.0f;		//��ѡ


	//���ز���(msaa)���ã������ڿ���ݣ���Ҫ������Ӧ��GPU����
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//���ö��ز���
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;				//��ѡ
	multisampling.pSampleMask = nullptr;				//��ѡ
	multisampling.alphaToCoverageEnable = VK_FALSE;		//��ѡ
	multisampling.alphaToOneEnable = VK_FALSE;			//��ѡ

	//��Ⱥ�ģ�����
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	

	//������ɫ�������

	//VkPipelineColorBlendAttachmentState��ÿ���󶨵�֡������е�������ɫ�������	
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	//������ɫ���,�Դﵽ��͸����Ч��
	colorBlendAttachment.blendEnable = VK_TRUE;

	//VkPipelineColorBlendStateCreateInfoȫ�ֵ���ɫ�������
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//����λ����Ļ�Ϸ�ʽ������colorBlendAttachment�ж������ɫ��Ϸ�ʽ���������logicOpEnable�������е�VkPipelineColorBlendAttachmentState���ᱻ����
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;				//��ѡ
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;					//��ѡ
	colorBlending.blendConstants[1] = 0.0f;					//��ѡ
	colorBlending.blendConstants[2] = 0.0f;					//��ѡ
	colorBlending.blendConstants[3] = 0.0f;					//��ѡ

	//��̬״̬
	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_LINE_WIDTH};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;


	//���߲���
	//�����ֿ�����������shader�е�uniform����	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;						//����������������
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;		//������������
	pipelineLayoutInfo.pushConstantRangeCount = 0;				//��ѡ
	pipelineLayoutInfo.pPushConstantRanges = nullptr;			//��ѡ

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("�������߲���ʧ��");
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
	pipelineInfo.pDepthStencilState = nullptr;				//��ѡ
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;					//��̬״̬����ѡ

	pipelineInfo.layout = pipelineLayout;				//���߲���
	pipelineInfo.renderPass = renderPass;				//��Ⱦpass
	//��sub����
	pipelineInfo.subpass = 0;
	//ָ���̳е������Ѿ����ڵĹ��ߣ���ѡ��
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;		//��ѡ
	//ָ���Ľ�Ҫ�����Ĺ���Ϊ�����ߣ���ѡ��
	pipelineInfo.basePipelineIndex = -1;					//��ѡ

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("����ͼ����Ⱦ����ʧ��");
	}
	
	shaderStages.clear();
	//��������shaderģ��(�ֽڷ�װ)
	vkDestroyShaderModule(device, fragShader, nullptr);
	vkDestroyShaderModule(device, vertShader, nullptr);
}

//����shaderģ��
VkShaderModule VulkanBase::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	//�贫���ֽڱ������ݵ�ָ�루����uint32_t*����ʹ��C++��reinterpret_cast��ת������
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("����shader ģ��ʧ��");
	}

	return shaderModule;
}

//�������߲���������
void VulkanBase::createDescriptorSetLayout()
{
	//uniform������
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	//�������������ͣ����ﶨ��������shader����uniform buffer.
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//����������
	uboLayoutBinding.descriptorCount = 1;
	//shader�׶�Ϊ������ɫ��
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	//ͼ��������������
	uboLayoutBinding.pImmutableSamplers = nullptr;//��ѡ

	//��������������
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	//�������������ͣ����ͼ�������
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;//��ѡ
	//shader�׶�ΪƬԪ��ɫ����������ɫ����Ҳ���Զ�ȡ��������ͨ�������򶥵���ɫ���д������ݣ������������ƬԪ��ɫ���в������������
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("��������������ʧ��");
	}
}

#pragma endregion


#pragma region ֡�����������



//����֡������
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
			throw std::runtime_error("����֡����ʧ��");
		}
	}
}

//���������
void VulkanBase::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//����������Ķ���
	commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.has_value() ? queueFamilyIndices.graphicsFamily.value() : -1;
	commandPoolInfo.flags = 0;			//��ѡ

	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("����������ʧ��");
	}
}


//���������
void VulkanBase::createCommandBuffer()
{
	commandBuffers.resize(swapChainFrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	//level����ָ�������ָ����������Ҫָ�������Ǹ���ָ������
	//VK_COMMAND_BUFFER_LEVEL_PRIMARY����ִ��ָ����Ա��ύ�����н���ִ�У������ܱ�����ָ���������
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY������ָ�����ֱ�ӱ��ύ�����н���ִ�У������Ա���Ҫָ���������ִ�С�
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data() )!= VK_SUCCESS)
	{
		throw std::runtime_error("���������ʧ��");
	}

	//��¼��������
	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//����ʹ�������
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT�����ύһ��
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT��ֻ��һ����Ⱦ������ʹ�õĸ�������塣
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT���������ȴ�ִ��ʱ��ͬʱ�����ύ��һ����塣
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	//��ʹ�����ǿ�������һ֡��δ������Ⱦʱ���ύ��һ֡����Ⱦָ��
		//beginInfo.pInheritanceInfo = nullptr;							//��ѡ

		//��ʼ¼������
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("��ʼ¼�������ʧ��");
		}
		{
			//���ÿ�ʼ��Ⱦpass
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFrameBuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			//ʹ����ȫ��͸���ĺ�ɫ��Ϊ���ֵ
			VkClearValue clearColor = { 0.1f, 0.2f, 0.5f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
			//VK_SUBPASS_CONTENTS_INLINE������Ҫִ�е�ָ�����Ҫָ����У�û�и���ָ�����Ҫִ��
			//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS�������Ը���ָ����ָ����Ҫִ��
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			{
				//��ͼ�ι���
				//�ڶ���������ָ�����߶�����ͼ�ι��߻��Ǽ������,VK_PIPELINE_BIND_POINT_GRAPHICS��ʾ��ͼ�ι���
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

				//�󶨶��㻺��
				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

				//����������
				vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

				//����������				
				vkCmdBindDescriptorSets(commandBuffers[i],
					VK_PIPELINE_BIND_POINT_GRAPHICS, //��������������ͼ�ι��������еģ���ͼ�ι���һ��������Ҫָ��Ҫ�󶨵���ͼ�ι��߻��Ǽ������
					pipelineLayout, 
					0,									//��Ҫ�󶨵����������ĵ�һ��Ԫ������
					1,									//��Ҫ�󶨵�������������
					&descriptorSets[i],					//������������
					0,									//ָ����̬������������ƫ��
					nullptr
				);

				//DrawCall����
				//vertexCount��������
				//instanceCount������ʵ����Ⱦ��Ϊ1 ʱ��ʾ������ʵ����Ⱦ��
				//firstVertex�����ڶ�����ɫ������gl_VertexIndex ��ֵ
				//firstInstance�����ڶ�����ɫ������gl_InstanceIndex ��ֵ
				//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
				//���ƴ�������mesh
				vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			}

			//������Ⱦ����
			vkCmdEndRenderPass(commandBuffers[i]);
		}
		//����¼������
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("¼�������ʧ��");
		}
	}


	
}

#pragma endregion

#pragma region ��Ⱦ����ʾ
//�����ź����͹�դ����,�����첽�����е�ͬ���ź�
//�ֱ���ͼ�λ�ȡ����Ⱦ�������ź�����һ������֪ͨ��ʼ��Ⱦ����һ������֪ͨ��ʼ��ʾ
void VulkanBase::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//���ó�ʼ״̬Ϊ�ѷ����źŵ�״̬������vkWaitForFences�������յ��źţ�����Զ�ȴ���ȥ
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{

		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("�����ź���ʧ��");
		}
	}

	
}

//�������߲��ֵ�unifrom����
void VulkanBase::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		createBuffer(bufferSize, 
			//������;��Uniform buffer
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT����cpuд�����ݣ��� VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ����֤�Դ�ɼ���һ���ԣ���֤�ڸ�gpu�ں��е�����һ����
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

//������������
void VulkanBase::createDescriptorPool()
{
	//��������size����
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	//��������size�ĸ���
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	////��������size����
	poolInfo.pPoolSizes = poolSizes.data();
	//����������������
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
	//�Ƿ���Ա������
	//poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("������������ʧ��");
	}


}

//��������������д������
void VulkanBase::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	//���������ֶ������Ҫƥ�����������������
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	//����������������������������������Destroyʱ�Զ���������Բ�����ʽ�ĵ���Destroy
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("������������ʧ��");
	}

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		//�������������õĻ������
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		//�����������ʹ��VK_WHOLE_SIZE���ʾʹ����������
		bufferInfo.range = sizeof(UniformBufferObject);

		//��ͼ��Ͳ���������������
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		//д�����ݵ���������
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//Ҫд���������������
		descriptorWrites[0].dstSet = descriptorSets[i];
		//�������������ΪdstSet���Ǿ����飬�����±�0��ʾ��һ��������������
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		//���������ͣ�uniform����
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//������������
		descriptorWrites[0].descriptorCount = 1;
		//����������Ҫд�뻺�����
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		//����������Ҫд���ͼ������
		descriptorWrites[0].pImageInfo = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		//���������ͣ�ͼ�������
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		//ִ��д��
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

//���ƣ�ÿ֡����
void VulkanBase::drawFrame()
{
	//�ȴ�һ���դ(fence) �е�һ����ȫ����դ(fence) �����ź�,waitall����ΪVK_TRUE�ڴ˴������壬��Ϊֻ��һ����դ
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	//�յ���դ�źź����ù�դ����Ϊδ�����źŵ�״̬
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	//�ӽ�������ȡͼƬ,KHR��׺�Ķ����뽻������صĲ���
	uint32_t imageIndex;
	//�˲������첽�ģ���4��5����Ϊ��ȡͼ����ɺ�֪ͨ�Ķ�������֪ͨ��ǰ֡ͼƬ�ź����͵�ǰ֡�Ĺ�դ���󣬴˴����Բ��ô����դ����
	//���һ�������Ƕ�ȡ��ͼƬ������Ϊ������ͼƬ�����������
	//����ֵ��ʾ�������Ƿ����
	auto result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], inFlightFences[currentFrame], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//����uniform����
	updateUniformBuffer(imageIndex);

	//�ύͼƬ��ָ���
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//Ҫ�ȴ����ź�������
	VkSemaphore waitSemaphore[] = {imageAvailableSemaphores[currentFrame]};
	//��Ҫ�ȴ��Ĺ��߽׶�,����������Ҫд����ɫ���ݵ�ͼ��������Ҫ�ȴ�ͼ����ߵ������д����ɫ���ŵĹ��߽׶�
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	//waitStages �����е���Ŀ��pWaitSemaphores����ͬ�������ź������Ӧ��
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	//�ύִ�е�ָ������
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	//����ָ�������֪ͨ���ź���
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	// �ύ��ͼ���жӣ����һ������Ϊ��դ����(��ѡ)������ָ��������Ĺ�դ������
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	
	//��ʾ
	VkPresentInfoKHR presendInfo = {};
	presendInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presendInfo.waitSemaphoreCount = 1;
	presendInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapChain};
	presendInfo.swapchainCount = 1;
	presendInfo.pSwapchains = swapChains;
	presendInfo.pImageIndices = &imageIndex;
	presendInfo.pResults = nullptr;		//��ѡ��ÿ���������ĳ��ֲ����Ƿ�ɹ�����Ϣ����������ֻʹ����һ�������������ֺ����ķ���ֵ���жϳ��ֲ����Ƿ�ɹ���û�б�Ҫʹ��pResults
	//��ʾ
	result = vkQueuePresentKHR(presentQueue, &presendInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	//���µ�ǰ֡,֡����0-MAX_FRAMES_IN_FLIGHT��ѭ��
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//����uniform����
void VulkanBase::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//ģ�͵���Ⱦ��Ƴ���Z ����Ⱦtime ����
	UniformBufferObject ubo = {};
	//glm::rotate �����Ծ�����ת�ǶȺ���ת����Ϊ����,glm::mat4(1.0f)Ϊ����ĵ�λ���� time * glm::radians(90.0f) ���ÿ����ת90 �ȵĲ�����
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::lookAt �����Թ۲���λ�ã��ӵ��������������Ϊ����������ͼ�任����
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//glm::perspective ����������Ĵ�ֱ�Ƕȣ�����Ŀ�߱��Լ���ƽ���Զƽ�����Ϊ��������͸�ӱ任����
	//��Ҫע���ڴ��ڴ�С�ı��Ӧ��ʹ�õ�ǰ��������Χ�����¼����߱�
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	//GLM �������ΪOpenGL ��Ƶģ����Ĳü������Y ���Vulkan���෴�ġ���ͶӰ�����Y ������ϵ������ȡ����ʹͶӰ�����Vulkan ��Ҫ��һ�¡�
	ubo.proj[1][1] *= -1;

	//�������ݵ���ǰ֡��Ӧ��uniform ����
	void* data;
	vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	{
		memcpy(data, &ubo, sizeof(ubo));
	}
	vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

#pragma endregion

#pragma region ͼƬ�����뻺��
//����������ͼ
void VulkanBase::createTextureImageView() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM);
}

//����������������Ʒ������VkImage�����ǿ������ڷ����κ�VkImage����
void VulkanBase::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//ָ��������Ҫ�Ŵ����Сʱ�Ĳ�ֵ������Ŵ����ֲ������ܵ����⣬������С����ֲ������������
	//VK_FILTER_NEAREST��VK_FILTER_LINEAR�ֱ��Ӧ�������
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	//Ѱַģʽ��uvw�ֱ��Ӧxyz
	//VK_SAMPLER_ADDRESS_MODE_REPEAT����������ͼ��Χʱ�ظ�����
	//VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT����������ͼ��Χʱ�ظ�����������
	//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE����������ͼ��Χʱʹ�þ�������ı߽�����
	//VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE����������ͼ��Χʱʹ�þ�����������ı߽�����
	//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER����������ͼ�񷵻�ʱ�������õı߽���ɫ
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	//�����������Թ���
	samplerInfo.anisotropyEnable = VK_TRUE;
	//���������������������һ�����Ϊ16�����������ܳɷ��ȣ���Ʒ�ʳ�����
	samplerInfo.maxAnisotropy = 16;
	//����VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDERѰַģʽʱ�ı߽���ɫ
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	//����ϵͳ��trueʱΪ0-width��0-height��ΪfalseʱΪ0-1��һ��Ϊfalse,��shader�е�һ��
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	//���ñȽϣ���������Ӱ�ȣ�
	samplerInfo.compareEnable = VK_FALSE;
	//�ȽϺ�Ĳ�������������Ӱ�ȣ�
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	//mipmapMode��mipLodBias��minLod ��maxLod ��Ա�����������÷ּ�ϸ��(mipmap)
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("�������������ʧ��");
	}
}

//����������
void VulkanBase::createTextureImage()
{
	//��ȡ����ͼƬ�ߴ��ʵ�ʴ��ڵ���ɫͨ��
	int texWidth, texHeight, texChannels;
	//STBI_rgb_alphaΪrgba��ͨ����ÿ������4���ֽڣ��ܳߴ�Ϊwidth * height * 4�ֽ�
	stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels)
	{
		throw std::runtime_error("��ȡͼƬtextures/texture.jpgʧ��");
	}

	//��ʱ��������cpu �ɼ�����
	VkBuffer tempBuffer;
	VkDeviceMemory tempBufferMemory;
	createBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,	//��Ϊ������Դ
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //��CPU д������ �� ���ָ�GPU�ں˵�һ����
		tempBuffer, tempBufferMemory);

	//����ͼƬ����ʱ������
	void *data;
	vkMapMemory(device, tempBufferMemory, 0, imageSize, 0, &data);
	{
		memcpy(data, pixels, static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(device, tempBufferMemory);
	//������ص�ͼƬ��cpu�ڴ��Ѿ�����Ҫ�ˣ�
	stbi_image_free(pixels);

	//������ʽ����
	createImage(texWidth, texHeight, 
		VK_FORMAT_R8G8B8A8_UNORM, //STBI_rgb_alpha���Ͷ�Ӧ�ĸ�ʽ
		//tilingֵ ��VK_IMAGE_TILING_LINEAR��������������ķ�ʽ���У����Ҫֱ�ӷ���ͼƬ���ݣ�����������Ϊ��
		//tilingֵ ��K_IMAGE_TILING_OPTIMAL��������һ�ֶԷ����Ż��ķ�ʽ����,��ʱ����Ȳ���Ҫ�����ͼƬ����Ϊ�˸�������
		VK_IMAGE_TILING_OPTIMAL,
		//����Ŀ�� �� ��shader����
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		textureImage, textureImageMemory);

	//ת��ͼ�񲼾֣�
	//��һ��ת����Ŀ�꣺�����ȡͼ��������Ϊ����Ŀ�ģ���������ִ�и���
	transitionImageLayout(textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_UNDEFINED,	//old layout��VK_IMAGE_LAYOUT_UNDEFINED��ʾ����Ҫ����֮ǰ�����ݣ����ܸ���
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL	//new layout ������Ŀ��
	);
	//ִ�и��ƣ�����ʱ���嵽��ʽ����
	copyBufferToImage(tempBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	//�ڶ���ת����Ŀ�꣺����shader��ȡ��������ƬԪ��ɫ���в��ܶ�ͼƬ���в���
	transitionImageLayout(textureImage, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		//old layout������Ŀ��
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	//new layout shader�ж�ȡ
	);

	vkDestroyBuffer(device, tempBuffer, nullptr);
	vkFreeMemory(device, tempBufferMemory, nullptr);

}
//���ߣ�����ͼ�񻺳���
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
		throw std::runtime_error("����ͼ�λ���ʧ��");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("����ͼ�λ�����ڴ�����ʧ��");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

//���ߣ��任ͼ�񲼾�
void VulkanBase::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	//��ͼ�񲼾ֽ��б任
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	//��Դ������������VK_QUEUE_FAMILY_IGNORED��ʾ���Զ���������Ȩ
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//Ŀ�������������VK_QUEUE_FAMILY_IGNORED��ʾ���Զ���������Ȩ
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//���в��ֱ任��ͼ���ͼ�����������
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	//����������֮ǰ�Ĺ��߽׶�
	VkPipelineStageFlags sourceStage;
	//����������֮��Ĺ��߽׶�
	VkPipelineStageFlags destinationStage;	
	
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		//��һ�α任ʱ������δ���嵽����Ŀ��
		//��Դ��δ����
		barrier.srcAccessMask = 0;
		//Ŀ�ģ�д��
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//�ϸ��׶Σ���ʼ���߽׶Σ����⣩
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		//Ŀ��׶Σ�����׶Σ����⣩
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		//�ڶ��α任ʱ�����Ӵ���Ŀ�ĵ�shader��ȡ
		//��Դ��д��
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//Ŀ�ģ�shader��ȡ
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//�ϸ��׶Σ�����׶Σ����⣩
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		//Ŀ��׶Σ�ƬԪ��ɫ������ʵ��
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	//��������ָ�����ͬ����Դ���ʣ��������ڱ�֤ͼ���ڱ���ȡ֮ǰ���ݱ�д��Ҳ���Ա������任ͼ�񲼣�Ҳ���Ա������任ͼ�񲼾�
	//�任ͼ�񲼾ֵ�ָ��
	vkCmdPipelineBarrier(
		commandBuffer,		
		sourceStage,		//����������֮ǰ�Ĺ��߽׶�
		destinationStage,	//����������֮��Ĺ��߽׶�
		0,					//dependencyFlags,����ΪVK_DEPENDENCY_BY_REGION_BIT �Ļ������Ͼͱ����һ����������
		0,					//memoryBarrierCount	�ڴ���
		nullptr,			//pMemoryBarriers
		0,					//bufferMemoryBarrierCount �����ڴ�����
		nullptr,			//pBufferMemoryBarriers
		1,					//imageMemoryBarrierCount ͼ������
		&barrier			//pImageMemoryBarriers
	);

	endSingleTimeCommands(commandBuffer);
}

//���ƻ��嵽ͼ��
void VulkanBase::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	//��������Ϊ�������ڴ��еĴ�ŷ�ʽ������Ϊ0Ϊ��������
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
//���ߣ���ʼ¼�Ƶ�������
VkCommandBuffer VulkanBase::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//����Ϊ��ִ��ָ��
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//���ύһ��,����ֻ��ִ��һ��
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//��ʼ¼��
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}
//���ߣ�����¼�Ƶ�������
void VulkanBase::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	//�ύ����嵽ͼ�ζ���
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//��դ����Կգ������첽�ȴ�
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	//�ȴ�������ɣ����դ����fence��һ�������ڽ���ִ��һ�Σ�������ͬ������������
	vkQueueWaitIdle(graphicsQueue);

	//������������
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//���ߣ�����������ͼ
VkImageView VulkanBase::createImageView(VkImage image, VkFormat format) {

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	//ָ��ͼ�����;��ͼ�����һ���ֿ��Ա����ʡ���������ǵ�ͼ��������ȾĿ�꣬����û��ϸ�ּ���ֻ����һ��ͼ��,VR������ܻ��ж��ͼͼ�㡣
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
		throw std::runtime_error("����������ͼʧ��!");
	}

	return imageView;
}

#pragma endregion

#pragma region ����������GPU����
//��ȡ����ģ�͡�������������
void VulkanBase::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();;

	//��ʱ����(gpu)
	//��ʱ���潫����CPU���Է��ʵĻ������ͣ���cpu �ڴ渴�Ƶ��˻�����
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize,		
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,	//������Դ
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,////��Ҫλ������VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT����cpuд�����ݣ��� VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ����֤�Դ�ɼ���һ���ԣ���֤�ڸ�gpu�ں��е�����һ����
		stagingBuffer,
		stagingBufferMemory
	);

	void *data;
	//�ڴ�<->�Դ�ӳ��,��VKDeviceMemoryӳ�䵽CPU�ڴ�
	//offet��size����ָ�����ڴ�ƫ�����ʹ�С������һ������ֵVK_WHOLE_SIZE ��������ӳ������������ڴ�
	//flagsΪԤ���ı�ǣ���δ�õ���������0
	//dataΪ�ڴ�ӳ���ĵ�ַ
	//��Ȼ�ڴ�����������ڴ浽�Դ�ĸ��ƣ�������gpu�ں��в�һ�������ɼ���Ҳ��һ��ͬʱ�ɼ�����Ϊ�ִ����������л�����һ���ơ�ǰ��ѡ���Դ�����ʱҪ����VK_MEMORY_PROPERTY_HOST_COHERENT_BIT����Ϊ�˱�����ں����ݲ�һ����
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	{
		//��cpu�ڴ����ݸ��Ƶ�ӳ���ַ��
		memcpy(data, vertices.data(), (size_t)bufferSize);
	}
	//�����ڴ�<->�Դ��ӳ��
	vkUnmapMemory(device, stagingBufferMemory);

	//��ʽ����(gpu)��CPU�޷����ʣ���GPU���ʵ����ܸ���
	createBuffer(bufferSize,
		//ָ���������ݵ�ʹ��Ŀ�ģ�������λ������ָ�������
		//VK_BUFFER_USAGE_TRANSFER_SRC_BIT �����ڴ洫�������������Դ
		//VK_BUFFER_USAGE_TRANSFER_DST_BIT �����ڴ洫�����������Ŀ��
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,//����Ŀ���붥����
		//����Ϊ�豸�ڲ��ڴ棬CPU�޷����ʣ���GPU���ʵ����ܸ���
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	//������ʱ���浽��ʽ���棨ͨ��GPU������壩
	copyBUffer(stagingBuffer, vertexBuffer, bufferSize);

	//�����ʱ����,copyBUffer��ͬ��ִ�еģ����ǲ��ù�դ������ź������첽������������ʱ�Ѿ�����˸���������ύ��
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

}

//��������
void VulkanBase::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	//��ʱ����
	VkBuffer tempBuffer;
	VkDeviceMemory tempBufferMemory;
	createBuffer(bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,//ʹ��Ŀ�ģ���������Դ
		//��Ҫλ������VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT����cpuд�����ݣ��� VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ����֤�Դ�ɼ���һ���ԣ���֤�ڸ�gpu�ں��е�����һ����
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		tempBuffer, tempBufferMemory
	);

	//CPU����ʱ������ڴ�ӳ��
	void *data;
	vkMapMemory(device, tempBufferMemory, 0, bufferSize, 0, &data);
	{
		memcpy(data, indices.data(), (size_t)bufferSize);
	}
	vkUnmapMemory(device, tempBufferMemory);

	//������ʽ����
	createBuffer(bufferSize,
		//ʹ��Ŀ�ģ�����Ŀ������������
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,indexBufferMemory
	);

	copyBUffer(tempBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, tempBuffer, nullptr);
	vkFreeMemory(device, tempBufferMemory, nullptr);
}

//���ߣ���������
void VulkanBase::createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	//ָ���������ݵ�ʹ��Ŀ�ģ�������λ������ָ�������
	bufferInfo.usage = usage;
	//ָ��������Ĺ���ģʽ������ֻ��һ�����У�����Ϊ��ռģʽ��
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	//���ص�����
	//��Ҫ���ڴ�ߴ�
	//memRequirements.size
	//�ڴ�����ʼλ��
	//memRequirements.alignment
	//ָʾ�ʺϵ��ڴ����͵�λ��
	//memRequirements.memoryTypeBits
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	//�����ڴ�
	//todo::�Կ��Է����ڴ�ĵ��ô����������Ƶģ�һ��������һ���Դ���һ���ϴ���ڴ棬Ȼ����Ӧ�ó����������з����ÿ������Ҳ��ʹ��GPUOpen��VulkanMemoryAllocator�ڴ������
	//��demo����ʱʹ��vkAllocateMemory���ǲ�������ʵ����Ŀ�е�
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;	
	//���������豸��֧�ֵ��Դ�����
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	//���Դ������뻺�����
	//���ĸ�������ƫ��ֵ����Ҫ�ܱ�memRequirements.alignment����
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
//���ߣ����ƻ������ݣ�ͨ�������
void VulkanBase::copyBUffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);

	////���������
	//VkCommandBufferAllocateInfo allocInfo = {};
	//allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//����Ϊ��ִ��ָ��
	//allocInfo.commandPool = commandPool;
	//allocInfo.commandBufferCount = 1;
	//VkCommandBuffer commandBuffer;
	//vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
	////¼������
	//VkCommandBufferBeginInfo beginInfo = {};
	//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	////���ύһ��,����ֻ��ִ��һ��
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
	////�ύ����嵽ͼ�ζ���
	//VkSubmitInfo submitInfo = {};
	//submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &commandBuffer;
	////��դ����Կգ������첽�ȴ�
	//vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	////�ȴ�������ɣ����դ����fence��һ�������ڽ���ִ��һ�Σ�������ͬ������������
	//vkQueueWaitIdle(graphicsQueue);
	////������������
	//vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//���ߣ�ѡ����ʵ��Դ�����
uint32_t VulkanBase::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	//�������飬���һ�����õ��ڴ�����
	//��Ӧλ��Ϊ1	
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("δ�ҵ����ʵ��Դ�����");
}
#pragma endregion



