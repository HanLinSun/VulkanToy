#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set=1, binding=1) uniform sampler2D diffuseTexture;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoords;
layout(location = 0) out vec4 outColor;

void main() {
   vec3 textureCol =texture(diffuseTexture, fragTexCoords).rgb;
   if(textureCol ==vec3(0.0))
   {
      outColor = vec4(1,1,1,1);
   }else outColor = vec4(textureCol.rgb,1);

}