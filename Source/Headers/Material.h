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
        , AmbientTextureIdx(-1)
        , EmissiveTextureIdx(-1)
        , DiffuseTextureIdx(-1)
        , SpecularTextureIdx(-1)
        , SpecularPowerTextureIdx(-1)
        , NormalTextureIdx(-1)
        , BumpTextureIdx(-1)
        , OpacityTextureIdx(-1)
        ,AlbedoTextureIdx(-1)
        ,MetallicTextureIdx(-1)
        ,RoughnessTextureIdx(-1)
        ,ReflectionTextureIdx(-1)
    {}
		glm::vec4 Diffuse ;
		glm::vec4 Specular ;

		glm::vec4 Ambient ;
		glm::vec4 Emissive ;
		glm::vec4 Reflectance ;
        
        glm::float32_t AlphaThreshold;
        glm::float32_t Opacity;                       // If Opacity < 1, then the material is transparent.
        glm::float32_t SpecularPower;
        glm::float32_t IndexOfRefraction;             // For transparent materials, IOR > 0.
        glm::float32_t BumpIntensity;                 // When using bump textures (height maps) will need to scale the height values so the normals are visible.

       glm::int32_t AmbientTextureIdx;
       glm::int32_t EmissiveTextureIdx;
       glm::int32_t DiffuseTextureIdx;
       glm::int32_t SpecularTextureIdx;

       glm::int32_t SpecularPowerTextureIdx;
       glm::int32_t NormalTextureIdx;
       glm::int32_t BumpTextureIdx;
       glm::int32_t OpacityTextureIdx;


       glm::float32_t Roughness;
       glm::float32_t ClearCoatRoughness;
       glm::float32_t ClearCoatThickness;
       glm::float32_t Metallic;
       glm::float32_t Sheen;

       glm::int32_t AlbedoTextureIdx;
       glm::int32_t RoughnessTextureIdx;
       glm::int32_t MetallicTextureIdx;
       glm::int32_t ReflectionTextureIdx;

};

enum TextureType
{
    Ambient = 0,
    Emissive=1,
    Diffuse=2,
    Specular=3,
    SpecularPower=4,
    Normal=5,
    Bump=6,
    Opacity=7,
    NumTypes=8,
    Albedo=9,
    Roughness=10,
    Metallic=11,
    Reflection=12,
};

using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;

class Material
{
public:
	Material(const MaterialProperties& materialProperties = MaterialProperties());
	Material(const Material& copy);
    Material(const Material* copy);
	~Material(){}

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

    void SetTexture(int textureID, TextureType type);
    int GetTextureID(TextureType type);

    bool IsTransparent() const;

    const MaterialProperties& GetMaterialProperties() const;
    void SetMaterialProperties(const MaterialProperties& materialProperties);

    void DestroyVKResources();


    std::string m_name;

private:

	std::unique_ptr<MaterialProperties, void (*)(MaterialProperties*)> m_MaterialProperties;

	//void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);

};