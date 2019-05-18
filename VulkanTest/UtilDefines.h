#pragma once

//GLM_FORCE_DEPTH_ZERO_TO_ONEA��glm��ʹ��0-1�����ֵ��������Ĭ��opengl��-1 - 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE	
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <glm/glm.hpp>		//���Դ�����
#include <glm/gtc/matrix_transform.hpp>	//�仯�����
#include <glm/gtx/hash.hpp>
#include <chrono>			//��ʱ



//�ɲ��д����֡
const int MAX_FRAMES_IN_FLIGHT = 2;

//���崰�ڴ�С
const int WIDTH = 1280;
const int HEIGHT = 720;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

//surface֧�ֽ�������ϸ��
struct SwapChainSupportDetails {
	//������������
	VkSurfaceCapabilitiesKHR capabilities;
	//����֧�ֵĸ�ʽ
	std::vector<VkSurfaceFormatKHR> formats;
	//֧�ֵĳ���ģʽ
	std::vector<VkPresentModeKHR> presentModes;
};



//���嶥�����ݽṹ��ʹ����glm���е��������ͣ���Ϊ������ȫ����shader�е���������
//glm���е��������Ϳ��Է��ĵ�ʹ��memcpy���������ݵ�GPU
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}




////����һ������ģ�͵Ķ�������
//const std::vector<Vertex> vertices = {
//	{{-0.5f, -0.5f,0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//	{{0.5f, -0.5f,0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//	{{0.5f, 0.5f,0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//	{{-0.5f, 0.5f,0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
//
//	{{-0.5f, -0.5f,-0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//	{{0.5f, -0.5f,-0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//	{{0.5f, 0.5f,-0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//	{{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//};
////����һ������ģ�͵���������
//const std::vector<uint16_t> indices = {
//	0, 1, 2, 2, 3, 0,
//	4,5,6,6,7,4
//};
//

//����һ��Ҫ����shader uniform�Ľṹ��
//ʹ��glm�ж�������ͣ���Ϊ�˺�shader�е����ͱ���һ�������Է��ĵ�ʹ��memcpy���������ݵ�GPU
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//���ߣ���ȡ�ļ�����
static std::vector<char> readFile(const std::string& fileName)
{
	//ate�����ļ�β����ʼ��ȡ
	//binary���Զ����Ƶ���ʽ��ȡ�ļ�
	std::unique_ptr<std::ifstream> fl(new std::ifstream(fileName, std::ios::ate | std::ios::binary));
	if (!fl->is_open())
	{
		throw std::runtime_error("��ȡ�ļ�ʧ��");
	}

	//ʹ��ate ģʽ�����ļ�β����ʼ��ȡ����������ͨ����ȡλ�õĽӿڻ���ļ���С
	size_t fileSize = (size_t)fl->tellg();
	std::vector<char> buffer(fileSize);

	//���ļ�����ͷ������ȡ�����ļ�
	fl->seekg(0);
	fl->read(buffer.data(), fileSize);

	fl->close();
	return buffer;
}




//У���
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



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