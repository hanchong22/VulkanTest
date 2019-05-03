#version 450
//GLSL的vulkan扩展
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

//定义一个uniform的结构体，用于接收从C++中传入的数据
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;		//模型矩阵
    mat4 view;		//视图矩阵
    mat4 proj;		//投影矩阵
} ubo;

//输入值
//layout(location = x) 用于指定变量在顶点数据中的索引。
//对于64 位变量，比如dvec3 类型的变量，它占用了不止一个索引位,在定义这种类型的顶点属性变量之后的顶点变量，要注意索引号增加的并不是1
//详细内容查阅OpenGL 的官方文档
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;


//输出值，片元着色器中将以此作为输入值
layout(location = 0) out vec3 fragColor;

void main() {
	//gl_VertexIndex变量包含了当前顶点的索引
    //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    //fragColor = colors[gl_VertexIndex];
    //gl_Position = vec4(inPosition, 0.0, 1.0);
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    
}
