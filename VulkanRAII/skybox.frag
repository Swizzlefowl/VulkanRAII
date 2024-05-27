#version 450
layout(location = 0) in vec3 skyBoxUVW;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform samplerCube cubeSampler;

void main() {
     outColor = texture(cubeSampler, skyBoxUVW);
     gl_FragDepth = 1.0f;
}