#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 instance;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) flat out int instanceIndex;

struct uniformBuffer{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(binding = 0) uniform uuniformBuffer{
   uniformBuffer ubos[2];
}ubo;

layout( push_constant ) uniform constants
{
    layout(offset = 4) int index;
} PushConstants;

void main() {
    gl_Position = ubo.ubos[PushConstants.index].proj * ubo.ubos[PushConstants.index].view * ubo.ubos[PushConstants.index].model * vec4(inPos + instance, 1.0);
    fragColor = inColor;
    texCoord = inTexCoord;
    instanceIndex = gl_InstanceIndex % 3;
}