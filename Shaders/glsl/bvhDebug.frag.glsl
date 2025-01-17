#version 460

layout(location = 0) in vec3 fragColor;  // Color passed from the vertex shader
layout(location = 0) out vec4 outColor;  // Final output color


void main() {
    outColor = vec4(fragColor, 1.0);  // Set the output color (e.g., opaque green)
}