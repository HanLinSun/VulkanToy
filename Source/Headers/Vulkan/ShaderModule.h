#pragma once
#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

enum ShaderType
{
    Vertex,
    Fragment,
    Geometry,
    Tessellation,
    Compute
};

class Shader
{
public:
    Shader() = delete;
    Shader(ShaderType type, const std::vector<char>& code, VkDevice logicalDevice);
    Shader(ShaderType type,const std::string& filename, VkDevice logicalDevice);

    virtual ~Shader();
    ShaderType GetType();
    VkShaderModule GetShaderModule();
protected:
    VkShaderModule Create(const std::vector<char>& code, VkDevice logicalDevice);
    VkShaderModule Create(const std::string& filename, VkDevice logicalDevice);
    VkShaderModule m_shaderModule;
    ShaderType m_type;
};