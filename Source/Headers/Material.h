#pragma once
#include "Vulkan/Device.h"
#include "Vulkan/Texture.h"
#include <glm/glm.hpp>
#include <map>
#include <memory>

struct TestMaterial
{
    uint32_t type;
    glm::vec3 albedo;
    double refraction_index;
};

struct MaterialProperties
{

    MaterialProperties(
        const glm::vec4 diffuse = { 1, 1, 1, 1 },
        const glm::vec4 specular = { 1, 1, 1, 1 },
        const float specularPower = 128.0f,
        const glm::vec4 ambient = { 0, 0, 0, 1 },
        const glm::vec4 emissive = { 0, 0, 0, 1 },
        const glm::vec4 reflectance = { 0, 0, 0, 0 },
        const glm::vec4 transmittance = { 1,1,1,1 },
        const float opacity = 1.0f,
        const float indexOfRefraction = 0.0f,
        const float bumpIntensity = 1.0f,
        const float alphaThreshold = 0.1f,
        const float transmission =1.0f,
        const float roughness = 1.0f,
        const float sheen = 1.0f,
        const float clearcoat = 1.0f,
        const float clearcoatGloss = 1.0f,
        const float metallic = 1.0f,
        const float subsurface =1.0f,
        const float anisotropic = 1.0f
    )   
        : Diffuse(diffuse)
        , Specular(specular)
        , Emissive(emissive)
        , Ambient(ambient)
        , Reflectance(reflectance)
        , Transmittance(transmittance)
        , Opacity(opacity)
        , SpecularPower(specularPower)
        , IndexOfRefraction(indexOfRefraction)
        , BumpIntensity(bumpIntensity)
        , AlphaThreshold(alphaThreshold)
        , Transmission(transmission)
        , Roughness(roughness)
        , ClearCoat(clearcoat)
        , ClearCoatGloss(clearcoatGloss)
        , Sheen(sheen)
        , Metallic(metallic)
        , Anisotropic(anisotropic)
        , SubSurface(subsurface)
        , AmbientTextureIdx(-1)
        , EmissiveTextureIdx(-1)
        , DiffuseTextureIdx(-1)
        , SpecularTextureIdx(-1)
        , SpecularPowerTextureIdx(-1)
        , NormalTextureIdx(-1)
        , BumpTextureIdx(-1)
        , OpacityTextureIdx(-1)
        , AlbedoTextureIdx(-1)
        , MetallicRoughnessTextureIdx(-1)
        , ReflectionTextureIdx(-1)
    {}
		glm::vec4 Diffuse ;
		glm::vec4 Specular ;

		glm::vec4 Ambient ;
		glm::vec4 Emissive ;
		glm::vec4 Reflectance ;
        glm::vec4 Transmittance;
        
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
       glm::float32_t ClearCoat;
       glm::float32_t ClearCoatGloss;
       glm::float32_t Metallic;
       glm::float32_t Transmission;
     
       glm::float32_t Sheen;
       glm::float32_t SheenTint=0;
       glm::float32_t SpecularTint = 0;
       glm::float32_t SpecTrans=0;

       glm::float32_t Anisotropic;
       glm::float32_t SubSurface;

       glm::int32_t AlbedoTextureIdx;
       glm::int32_t MetallicRoughnessTextureIdx;
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
    MetallicRoughness=10,
    Reflection=11,
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

    glm::float32_t GetSpecularPower() const;
    void SetSpecularPower(glm::float32_t specularPower);

    const glm::float32_t GetOpacity() const;
    void SetOpacity(glm::float32_t opacity);

    glm::float32_t GetIndexOfRefraction() const;
    void SetIndexOfRefraction(glm::float32_t indexOfRefraction);

    glm::float32_t GetRoughness() const;
    void SetRoughness(glm::float32_t roughness);

    glm::float32_t GetMetallic() const;
    void SetMetallic(glm::float32_t metallic);

    glm::float32_t GetClearCoat() const;
    void SetClearCoat(glm::float32_t clearCoat);

    glm::float32_t GetClearCoatGloss() const;
    void SetClearCoatGloss(glm::float32_t ClearCoatGloss);

    glm::float32_t GetBumpIntensity() const;
    void SetBumpIntensity(glm::float32_t bumpIntensity);

    glm::float32_t GetSheen() const;
    void SetSheen(glm::float32_t sheen);

    glm::float32_t GetSheenTint() const;
    void SetSheenTint(glm::float32_t sheenTint);

    glm::float32_t GetSpecularTint() const;
    void SetSpecularTint(glm::float32_t specularTint);

    glm::float32_t GetSpecTrans() const;
    void SetSpecTrans(glm::float32_t specTrans);

    glm::float32_t GetAnisotropic() const;
    void SetAnisotropic(glm::float32_t anisotropic);

    glm::float32_t GetSubSurface() const;
    void SetSubSurface(glm::float32_t subsurface);

    glm::vec4 GetTransmittance() const;
    void SetTransmittance(glm::vec4 tranmittance);

    glm::float32_t GetTransmission() const;
    void SetTransmission(glm::float32_t transmission);


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