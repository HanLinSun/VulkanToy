// Fragment Shader
#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    // Sample the texture using the interpolated UV coordinates
    outColor = texture(texSampler, vec2(fragUV.s, 1.0 -fragUV.t));

   // outColor = vec4(1, 0, 0, 1);
}