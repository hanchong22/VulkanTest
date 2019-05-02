
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


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);



class VulkanBase
{
public:
	void Run();

public:
	//���ڴ�С�Ƿ�ı�
	bool framebufferResized = false;

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
	//���߲��֣�������shader����uniform����
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

	//�������ݻ������
	VkBuffer vertexBuffer;
	//���������Դ�����
	VkDeviceMemory vertexBufferMemory;
	//�����������
	VkBuffer indexBuffer;
	//���������ڴ�����
	VkDeviceMemory indexBufferMemory;

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
	//���������
	void cleanupSwapChain();
	//�ؽ�������
	void recreateSwapChain();
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
	//������������
	void createVertexBuffer();
	//������������
	void createIndexBuffer();


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
	//�����Դ�����
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	//����buffer
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	//���ƻ�������
	void copyBUffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	
};
