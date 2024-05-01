#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

layout( push_constant ) uniform constants
{
    int index;
} PushConstants;

layout(binding = 1) uniform sampler2D texSampler[2];

void main() {
    outColor = texture(texSampler[PushConstants.index], texCoord);
       
}