#version 450

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Sample the texture using the interpolated UV coordinates

    outColor = texture(texSampler, vec2(inUV.s,  inUV.t));
}
