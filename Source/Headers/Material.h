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

struct LambertMaterial
{
	    Device* m_device = nullptr;
	    AlphaBlendMode m_alphaMode = MODE_OPAQUE;

		float m_opticalDensity = 1.0f;
		float m_dissolve=0.0f;

		glm::vec3 m_ambientColor = glm::vec3(1.0f);
	    glm::vec3 m_diffuseColor = glm::vec3(1.0f);
		glm::vec3 m_specularColor = glm::vec3(1.0f);

	    Texture* m_baseColorTexture = nullptr;
	    Texture* m_metallicRoughnessTexture = nullptr;
	    Texture* m_normalTexture = nullptr;
	    Texture* m_occlusionTexture = nullptr;
	    Texture* m_emissiveTexture = nullptr;
	    
	    Texture* m_specularGlossinessTexture;
	    Texture* m_diffuseTexture;
	    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
};

struct PBRMaterial
{
	Device* device = nullptr;
	AlphaBlendMode alphaMode = MODE_OPAQUE;

	float alphaCutoff = 1.0f;
	float metallicFactor = 1.0f;
	float roughnessFactor = 1.0f;

	glm::vec4 baseColorFactor = glm::vec4(1.0f);

	Texture* baseColorTexture = nullptr;
	Texture* metallicRoughnessTexture = nullptr;
	Texture* normalTexture = nullptr;
	Texture* occlusionTexture = nullptr;
	Texture* emissiveTexture = nullptr;

	Texture* specularGlossinessTexture;
	Texture* diffuseTexture;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
};
