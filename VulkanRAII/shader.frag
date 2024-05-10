#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 skyBoxUVW;
layout(location = 0) out vec4 outColor;

layout( push_constant ) uniform constants
{
    int index;
} PushConstants;

layout(binding = 1) uniform sampler2D texSampler[3];
layout(binding = 2) uniform samplerCube cubeSampler;

void main() {
    //outColor = texture(texSampler[PushConstants.index], texCoord);
     outColor = texture(cubeSampler, skyBoxUVW);
}