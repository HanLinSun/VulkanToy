#version 450


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoords;
layout(location = 0) out vec4 outColor;

void main() {
 //  outColor = vec4(texture(texSampler, fragTexCoords).rgb,1);
     outColor = vec4(fragNormal,1);
}