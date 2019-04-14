
#include "VulkanBase.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int WIDTH = 1280;
const int HEIGHT = 720;

//У���
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};




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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanBase::initVulkan() {
	createInstance();		//����ʵ��
	setupDebugMessenger();	//����У���
	createSurface();		//�������ڱ���
	pickPhysicalDevice();	//ѡ�������豸�Ͷ�����
	createLogicalDevice();	//�߼��豸�Ͷ���
	createSwapChain();		//����������
	createImageViews();		//����ͼ����ͼ
	createGraphicsPipeline();	//����ͼ�ι���
}


void VulkanBase::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VulkanBase::cleanup() {
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

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
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
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

	//����glf����ɴ�����洴���������֧��windowsƽ̨
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

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//VK_SHARING_MODE_EXCLUSIVE��һ��ͼ��ͬһʱ��ֻ��ֻ�ܱ�һ����������ӵ�У�����һ������ʹ����֮ǰ��������ʽ�ظı�ͼ������Ȩ����һģʽ�����ܱ������
	//VK_SHARING_MODE_CONCURRENT��ͼ������ڶ���������ʹ�ã�����Ҫ��ʽ�ظı�ͼ������Ȩ��	
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

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	//ָ��alpha ͨ���Ƿ������ʹ���ϵͳ�е��������ڽ��л�ϲ���,���ｫ������ΪVK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR�����Ե�alpha ͨ����
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
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

//ѡ�񽻻���Χ
//������Χ�ǽ�������ͼ��ķֱ��ʣ����������Ǻ�����Ҫ��ʾͼ��Ĵ��ڵķֱ�����ͬ
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

#pragma endregion

#pragma region ͼ����ͼ
void VulkanBase::createImageViews() {
	//�����㹻������ռ����洢ͼ����ͼ
	swapChainImageViews.resize(swapChainImages.size());

	//�������н�����ͼ�񣬴���ͼ����ͼ
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		//ָ��ͼ�񱻿�����һά������ά������ά��������������ͼ
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		//����ͼ����ɫͨ����ӳ��,���ڵ�ɫ�������ǿ��Խ�������ɫͨ��ӳ�䵽��ɫͨ������Ҳ����ֱ�ӽ���ɫ	ͨ����ֵӳ��Ϊ����0 ��1,���������ֻʹ��Ĭ�ϵ�ӳ��
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//ָ��ͼ�����;��ͼ�����һ���ֿ��Ա����ʡ���������ǵ�ͼ��������ȾĿ�꣬����û��ϸ�ּ���ֻ����һ��ͼ��,VR������ܻ��ж��ͼͼ�㡣
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		//����vkCreateImageView ��������ͼ����ͼ
		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}
#pragma endregion

#pragma region ͼ�ι�����shader
//����ͼ�ι���
void VulkanBase::createGraphicsPipeline() {

	//��ȡshader�ֽڣ����ȡ������shader����ʹ��glslangValidator.exe����shader���룩
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");
	//����shaderģ�飨�ֽڷ�װ��
	auto vertShader = createShaderModule(vertShaderCode);
	auto fragShader = createShaderModule(fragShaderCode);

	//����shader�׶Σ�����ȷ����shader������(frag��vectex)��ִ�к���
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName = "main";
	//���´��������shader����define����Щdefineһ������shader�ڲ��ķ�֧#if #else #endif
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShader;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo ,fragShaderStageInfo };


	//��������
	//todo::��ȡ����(mesh)�ļ�����
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	//����װ��
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//����ͼԪ������
	//VK_PRIMITIVE_TOPOLOGY_POINT_LIST����ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_LINE_LIST��ÿ�������㹹��һ���߶�ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP��ÿ�������㹹��һ���߶�ͼԪ������һ���߶�ͼԪ�⣬ÿ���߶�ͼԪʹ����һ���߶�ͼԪ��һ������
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST��ÿ�������㹹��һ��������ͼԪ
	//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP��ÿ�������εĵڶ����͵��������㱻��һ����������Ϊ��һ�͵ڶ�������ʹ��
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//�Ƿ����ü���ͼԪ����
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//�ӿںͲü�
	//�ӿ�
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

#pragma endregion





