#pragma once
#include "Vulkan/Device.h"
#include "Vulkan/Texture.h"
#include <glm/glm.hpp>
enum AlphaBlendMode
{
	MODE_OPAQUE,
	MODE_MASK,
	MODE_BLEND,
};

class Material
{
public:
		Material(Device* device) : m_device(device) {};
		void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
protected:
	    Device* m_device = nullptr;
	    AlphaBlendMode m_alphaMode = MODE_OPAQUE;
	    float m_alphaCutoff = 1.0f;
	    float m_metallicFactor = 1.0f;
	    float m_roughnessFactor = 1.0f;
	    glm::vec4 m_baseColorFactor = glm::vec4(1.0f);
	    Texture* m_baseColorTexture = nullptr;
	    Texture* m_metallicRoughnessTexture = nullptr;
	    Texture* m_normalTexture = nullptr;
	    Texture* m_occlusionTexture = nullptr;
	    Texture* m_emissiveTexture = nullptr;
	    
	    Texture* m_specularGlossinessTexture;
	    Texture* m_diffuseTexture;
	    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
};
