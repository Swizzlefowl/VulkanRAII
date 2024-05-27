#version 450

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec3 skyBoxUVW;

layout(binding = 0) uniform uniformBuffer{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

void main() {
    skyBoxUVW = inPos;
    //gl_Position = ubo.proj * ubo.view * vec4(inPos, 1.0);
    vec4 pos = ubo.proj * ubo.view * vec4(inPos, 1.0);
    gl_Position = pos.xyww;
}