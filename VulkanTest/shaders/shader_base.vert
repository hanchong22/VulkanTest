#version 450
//GLSL��vulkan��չ
#extension GL_ARB_separate_shader_objects : enable

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

//����һ��uniform�Ľṹ�壬���ڽ��մ�C++�д��������
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;		//ģ�;���
    mat4 view;		//��ͼ����
    mat4 proj;		//ͶӰ����
} ubo;

//����ֵ
//layout(location = x) ����ָ�������ڶ��������е�������
//����64 λ����������dvec3 ���͵ı�������ռ���˲�ֹһ������λ,�ڶ����������͵Ķ������Ա���֮��Ķ��������Ҫע�����������ӵĲ�����1
//��ϸ���ݲ���OpenGL �Ĺٷ��ĵ�
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;


//���ֵ��ƬԪ��ɫ���н��Դ���Ϊ����ֵ
layout(location = 0) out vec3 fragColor;

void main() {
	//gl_VertexIndex���������˵�ǰ���������
    //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    //fragColor = colors[gl_VertexIndex];
    //gl_Position = vec4(inPosition, 0.0, 1.0);
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    
}
