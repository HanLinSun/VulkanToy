#version 450

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Sample the texture using the interpolated UV coordinates
    //outColor = texture(texSampler, vec2(fragUV.s, 1.0 -fragUV.t));

    outColor = vec4(1, 1, 0, 1);
}

//#version 450
//
//layout(location = 0) in vec3 fragColor;
//
//layout(location = 0) out vec4 outColor;
//
//void main() {
//    outColor = vec4(fragColor, 1.0);
//}