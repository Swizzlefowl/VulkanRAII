#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in int instanceIndex;
layout(location = 0) out vec4 outColor;

layout( push_constant ) uniform constants
{
    int index;
} PushConstants;

layout(binding = 1) uniform sampler2D texSampler[3];

void main() {
    outColor = texture(texSampler[0], texCoord);
}