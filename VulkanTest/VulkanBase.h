
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



//�ж�����ҽ��
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//��ѯ���Ľ�����ϸ����Ϣ
struct SwapChainSupportDetails {
	//������������
	VkSurfaceCapabilitiesKHR capabilities;
	//����֧�ֵĸ�ʽ
	std::vector<VkSurfaceFormatKHR> formats;
	//֧�ֵĳ���ģʽ
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
	//���ڱ���
	VkSurfaceKHR surface;

	//�����豸
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//�߼��豸
	VkDevice device;

	//����
	//ͼ�ζ��� 
	VkQueue graphicsQueue;
	//���ֶ���
	VkQueue presentQueue;

	//������
	VkSwapchainKHR swapChain;
	//������ͼ��
	std::vector<VkImage> swapChainImages;
	//�����������ʽ
	VkFormat swapChainImageFormat;
	//��������Χ
	VkExtent2D swapChainExtent;
	//ͼ����ͼ
	std::vector<VkImageView> swapChainImageViews;
	//��Ⱦ����
	VkRenderPass renderPass;
	//���߲���
	VkPipelineLayout pipelineLayout;
	//ͼ����Ⱦ����
	VkPipeline graphicsPipeline;
	//֡����
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	//�����
	VkCommandPool commandPool;
	//����壬����������������ر����ʱ�Զ������
	std::vector<VkCommandBuffer> commandBuffers;

	//���������ź���������Ⱦ����ʾ��һ��֪ͨͼ���ѱ���ȡ���Կ�ʼ��Ⱦ����һ��֪ͨ��Ⱦ�Ѿ��������Կ�ʼ��ʾ
	//ͼ���ѱ���ȡ���ź���
	std::vector<VkSemaphore> imageAvailableSemaphores;
	//֪ͨ��Ⱦ�Ѿ��������ź���
	std::vector<VkSemaphore> renderFinishedSemaphores;
	//��դ����,����CPU ��GPU֮���ͬ������ֹ�г���MAX_FRAMES_IN_FLIGHT֡��ָ��ͬʱ���ύִ��
	std::vector<VkFence> inFlightFences;
	//��ǰ��Ⱦ������һ֡
	size_t currentFrame = 0;

private :

	//��ʼ������
	void initWindow();
	//��ʼ��
	void initVulkan();
	//������ѭ��
	void mainLoop();
	//�����ڴ�
	void cleanup();
	//����vulkanʵ��
	void createInstance();
	//����У���
	void setupDebugMessenger();		
	//�������ڱ���
	void createSurface();
	//ѡ�������豸�Ͷ�����
	void pickPhysicalDevice();
	//�߼��豸�Ͷ���
	void createLogicalDevice();
	//����������
	void createSwapChain();
	//����ͼ����ͼ
	void createImageViews();
	//������ȾPass
	void createRenderPass();
	//����ͼ�ι���
	void createGraphicsPipeline();
	//����֡����
	void createFramebuffers();
	//���������
	void createCommandPool();
	//���������
	void createCommandBuffer();
	//����ͬ������
	void createSyncObjects();
	//����
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
	//����shaderģ��
	VkShaderModule createShaderModule(const std::vector<char>& code);

	//У���ص����ڴ���Ϣ�ڴ���vulkan�ص��Ĵ�����
	//ʹ��VKAPI_ATTR ��VKAPI_CALL ����,�������ܱ�Vulkan�����
	// messageSeverity Ϊ��Ϣ������������Щֵ ��
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT�������Ϣ
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT��	��Դ����֮�����Ϣ
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT��		������Ϣ
	//		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT��		���Ϸ��Ϳ�����ɱ����Ĳ�����Ϣ
	//messageType ����������������Щֵ��
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT��	������һЩ��淶�������޹ص��¼�
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT��	������Υ���淶�����������һ�����ܵĴ���
	//		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT��	�����˿���Ӱ��Vulkan ���ܵ���Ϊ
	//pCallbackData������һ��ָ��VkDebugUtilsMessengerCallbackDataEXT�ṹ���ָ��,������pMessage��һ����null ��β�İ���������Ϣ���ַ�����pObjects���洢�к���Ϣ��ص�Vulkan �����������飻objectCount�������еĶ������
	//pUserData��һ��ָ�����������ûص�����ʱ�����ݵ����ݵ�ָ��
	//������һ������ֵ��������ʾ����У��㴦���Vulkan API�����Ƿ��жϡ��������ֵΪtrue����ӦVulkan API ���þͻ᷵��VK_ERROR_VALIDATION_FAILED_EXT ������롣ͨ����ֻ�ڲ���У��㱾��ʱ�᷵��true����������£��ص�����Ӧ�÷���VK_FALSE��
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			//todo::����ϢΪ��Ҫ��Ϣ����Ҫ���ϲ���ʾ
		}

		return VK_FALSE;
	}
};
