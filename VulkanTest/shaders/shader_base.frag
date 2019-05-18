#version 450
#extension GL_ARB_separate_shader_objects : enable
//uniform 对象，即是从C++中通过描述符传入的对象
layout(binding = 1) uniform sampler2D texSampler;

//顶点着色器传入的对象
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	//outColor = vec4(fragColor, 1.0);
	outColor = vec4( (texture(texSampler, fragTexCoord ).rgb * fragColor ),1.0);
}