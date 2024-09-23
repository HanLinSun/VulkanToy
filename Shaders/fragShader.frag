#version 450

layout(set=1, binding=1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoords;
layout(location = 0) out vec4 outColor;

void main() {
   vec3 textureCol =texture(texSampler, fragTexCoords).rgb;
   if(textureCol ==vec3(0.0))
   {
      textureColor = fragNormal;
   }else outColor = vec4(textureCol,1);

}