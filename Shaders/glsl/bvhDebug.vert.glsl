#version 460

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ModelBufferObject {
    mat4 model;
};

layout(set = 2, binding = 0) buffer BVHBuffer {
    vec3 nodeMin[];
    vec3 nodeMax[];
};

// For each BVH node, draw the 12 edges of its bounding box
layout(location = 0) in int inNodeIndex; // Index into BVH buffer

layout(location = 0) out vec3 fragColor;

void main() {
    vec3 corners[8] = vec3[8](
        vec3(nodeMin[inNodeIndex].x, nodeMin[inNodeIndex].y, nodeMin[inNodeIndex].z),
        vec3(nodeMax[inNodeIndex].x, nodeMin[inNodeIndex].y, nodeMin[inNodeIndex].z),
        vec3(nodeMax[inNodeIndex].x, nodeMax[inNodeIndex].y, nodeMin[inNodeIndex].z),
        vec3(nodeMin[inNodeIndex].x, nodeMax[inNodeIndex].y, nodeMin[inNodeIndex].z),
        vec3(nodeMin[inNodeIndex].x, nodeMin[inNodeIndex].y, nodeMax[inNodeIndex].z),
        vec3(nodeMax[inNodeIndex].x, nodeMin[inNodeIndex].y, nodeMax[inNodeIndex].z),
        vec3(nodeMax[inNodeIndex].x, nodeMax[inNodeIndex].y, nodeMax[inNodeIndex].z),
        vec3(nodeMin[inNodeIndex].x, nodeMax[inNodeIndex].y, nodeMax[inNodeIndex].z)
        );

    // Define the edges of the cube
    int edges[24] = int[24](
        0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
        4, 5, 5, 6, 6, 7, 7, 4, // Top face
        0, 4, 1, 5, 2, 6, 3, 7  // Vertical edges
        );

    // Output position and color
    int edgeIndex = gl_VertexID / 2; // Each line has 2 vertices
    int cornerIndex = edges[edgeIndex];
    vec3 position = corners[cornerIndex];

    gl_Position = camera.proj * camera.view * model * vec4(position, 1.0);
    fragColor = vec3(1.0, 0.0, 0.0); // Red for debug
}