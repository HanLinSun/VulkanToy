#version 450

layout(location = 0) out vec2 fragUV;

void main() {
    // Hard-coded positions for a full-screen quad (NDC: Normalized Device Coordinates)
    vec2 positions[4] = vec2[4](
        vec2(-1.0, -1.0), // Bottom-left
        vec2(1.0, -1.0), // Bottom-right
        vec2(-1.0, 1.0), // Top-left
        vec2(1.0, 1.0)  // Top-right
        );

    vec2 uvs[4] = vec2[4](
        vec2(0.0, 0.0), // UV for bottom-left
        vec2(1.0, 0.0), // UV for bottom-right
        vec2(0.0, 1.0), // UV for top-left
        vec2(1.0, 1.0)  // UV for top-right
        );

    // Emit vertex position and UV based on gl_VertexIndex
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragUV = uvs[gl_VertexIndex];
}
