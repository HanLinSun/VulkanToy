#pragma once
#include "Vulkan/Device.h"
#include "Vulkan/Texture.h"
#include <glm/glm.hpp>
#include <map>
#include <memory>

struct MaterialProperties
{

    MaterialProperties(
        const glm::vec4 diffuse = { 1, 1, 1, 1 },
        const glm::vec4 specular = { 1, 1, 1, 1 },
        const float specularPower = 128.0f,
        const glm::vec4 ambient = { 0, 0, 0, 1 },
        const glm::vec4 emissive = { 0, 0, 0, 1 },
        const glm::vec4 reflectance = { 0, 0, 0, 0 }, 
        const float opacity = 1.0f,
        const float indexOfRefraction = 0.0f, 
        const float bumpIntensity = 1.0f,
        const float alphaThreshold = 0.1f,
        const float roughness = 1.0f,
        const float sheen =1.0f,
        const float clearcoatRoughness = 1.0f,
        const float clearcoatThickness=1.0f,
        const float metallic = 1.0f
    )   
        : Diffuse(diffuse)
        , Specular(specular)
        , Emissive(emissive)
        , Ambient(ambient)
        , Reflectance(reflectance)
        , Opacity(opacity)
        , SpecularPower(specularPower)
        , IndexOfRefraction(indexOfRefraction)
        , BumpIntensity(bumpIntensity)
        , AlphaThreshold(alphaThreshold)
        , Roughness(roughness)
        , ClearCoatRoughness(clearcoatRoughness)
        , ClearCoatThickness(clearcoatThickness)
        , Sheen(sheen)
        , Metallic(metallic)
        , HasAmbientTexture(false)
        , HasEmissiveTexture(false)
        , HasDiffuseTexture(false)
        , HasSpecularTexture(false)
        , HasSpecularPowerTexture(false)
        , HasNormalTexture(false)
        , HasBumpTexture(false)
        , HasOpacityTexture(false)
        ,HasAlbedoTexture(false)
        ,HasMetallicTexture(false)
        ,HasRoughnessTexture(false)
        ,HasReflectionTexture(false)
    {}
		glm::vec4 Diffuse ;
		glm::vec4 Specular ;

		glm::vec4 Ambient ;
		glm::vec4 Emissive ;
		glm::vec4 Reflectance ;
        

        float AlphaThreshold;
        float Opacity;                       // If Opacity < 1, then the material is transparent.
        float SpecularPower;
        float IndexOfRefraction;             // For transparent materials, IOR > 0.
        float BumpIntensity;                 // When using bump textures (height maps) will need to scale the height values so the normals are visible.

        uint32_t HasAmbientTexture;
        uint32_t HasEmissiveTexture;
        uint32_t HasDiffuseTexture;
        uint32_t HasSpecularTexture;

        uint32_t HasSpecularPowerTexture;
        uint32_t HasNormalTexture;
        uint32_t HasBumpTexture;
        uint32_t HasOpacityTexture;


        float Roughness;
        float ClearCoatRoughness;
        float ClearCoatThickness;
        float Metallic;
        float Sheen;

        uint32_t HasAlbedoTexture;
        uint32_t HasRoughnessTexture;
        uint32_t HasMetallicTexture;
        uint32_t HasReflectionTexture;



};

enum TextureType
{
    Ambient,
    Emissive,
    Diffuse,
    Specular,
    SpecularPower,
    Normal,
    Bump,
    Opacity,
    NumTypes,
    Albedo,
    Roughness,
    Metallic,
    Reflection,
};

using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;

class Material
{
public:
	Material(const MaterialProperties& materialProperties = MaterialProperties());
	Material(const Material& copy);
    Material(const Material* copy);


	~Material() = default;

    const glm::vec4 GetAmbientColor() const;
    void SetAmbientColor(const glm::vec4& ambient);

    const glm::vec4 GetDiffuseColor() const;
    void SetDiffuseColor(const glm::vec4& diffuse);

    const glm::vec4 GetEmissiveColor() const;
    void SetEmissiveColor(const glm::vec4& emissive);

    const glm::vec4 GetSpecularColor() const;
    void SetSpecularColor(const glm::vec4& specular);

    float GetSpecularPower() const;
    void  SetSpecularPower(float specularPower);

    const float GetOpacity() const;
    void SetOpacity(float opacity);

    float GetIndexOfRefraction() const;
    void SetIndexOfRefraction(float indexOfRefraction);

    float GetRoughness() const;
    void SetRoughness(float roughness);

    float GetMetallic() const;
    void SetMetallic(float metallic);

    float GetClearCoatRoughness() const;
    void SetClearCoatRoughness(float clearCoarRoughness);

    float GetClearCoatThickness() const;
    void SetClearCoatThickness(float clearCoatThickness);

    float GetBumpIntensity() const;
    void SetBumpIntensity(float bumpIntensity);

    void SetTexture(std::shared_ptr<Texture> texture, TextureType type);
    bool IsTransparent() const;

    const MaterialProperties& GetMaterialProperties() const;
    void SetMaterialProperties(const MaterialProperties& materialProperties);

    std::shared_ptr<Texture> GetTexture(TextureType type) const;

    std::string m_name;

private:

	std::unique_ptr<MaterialProperties, void (*)(MaterialProperties*)> m_MaterialProperties;
    TextureMap m_textures;

  
	//void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);

};