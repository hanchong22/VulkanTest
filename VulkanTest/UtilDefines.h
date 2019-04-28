#pragma once

#include <vector>
#include <memory>
#include <fstream>

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