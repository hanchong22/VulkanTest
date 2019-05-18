#pragma once

//GLM_FORCE_DEPTH_ZERO_TO_ONEA让glm库使用0-1的深度值，而不是默认opengl的-1 - 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE	
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <glm/glm.hpp>		//线性代数库
#include <glm/gtc/matrix_transform.hpp>	//变化矩阵库
#include <glm/gtx/hash.hpp>
#include <chrono>			//计时



//可并行处理的帧
const int MAX_FRAMES_IN_FLIGHT = 2;

//定义窗口大小
const int WIDTH = 1280;
const int HEIGHT = 720;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

//surface支持交换链的细节
struct SwapChainSupportDetails {
	//基础表面特性
	VkSurfaceCapabilitiesKHR capabilities;
	//表面支持的格式
	std::vector<VkSurfaceFormatKHR> formats;
	//支持的呈现模式
	std::vector<VkPresentModeKHR> presentModes;
};



//定义顶点数据结构，使用了glm库中的数据类型，因为可以完全兼容shader中的数据类型
//glm库中的数据类型可以放心的使用memcpy来复制数据到GPU
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




////定义一个网格模型的顶点数据
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
////定义一个网格模型的索引数据
//const std::vector<uint16_t> indices = {
//	0, 1, 2, 2, 3, 0,
//	4,5,6,6,7,4
//};
//

//定义一个要传入shader uniform的结构体
//使用glm中定义的类型，是为了和shader中的类型保持一至，可以放心的使用memcpy来复制数据到GPU
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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




//校验层
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};



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