D:\VulkanSDK\Bin\glslc.exe -fshader-stage=vertex vertShader.glsl -o VertexShader.spv 
D:\VulkanSDK\Bin\glslc.exe -fshader-stage=fragment fragShader.glsl -o FragmentShader.spv 
D:\VulkanSDK\Bin\glslc.exe -fshader-stage=vertex showTexture.vert.glsl -o showTexture.vert.spv 
D:\VulkanSDK\Bin\glslc.exe -fshader-stage=fragment showTexture.frag.glsl -o showTexture.frag.spv 

D:\VulkanSDK\Bin\glslc.exe rayTrace.comp -o rayTrace.spv
pause