#pragma once

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>		//���Դ�����

//�ɲ��д����֡
const int MAX_FRAMES_IN_FLIGHT = 2;

//���崰�ڴ�С
const int WIDTH = 1280;
const int HEIGHT = 720;

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

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

//����һ������ģ��
const std::vector<Vertex> vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
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