#include "Material.h"

static MaterialProperties* NewMaterialProperties(const MaterialProperties& props)
{
	MaterialProperties* materialProperties = (MaterialProperties*)_aligned_malloc(sizeof(MaterialProperties), 16);
	*materialProperties = props;
	return materialProperties;
}

// Aligned memory must be deleted using the _aligned_free method.
static void DeleteMaterialProperties(MaterialProperties* p)
{
	_aligned_free(p);
}

Material::Material(const MaterialProperties& materialProperties):m_MaterialProperties(NewMaterialProperties(materialProperties),&DeleteMaterialProperties){}

Material::Material(const Material& copy):m_MaterialProperties(NewMaterialProperties(*copy.m_MaterialProperties), &DeleteMaterialProperties)
,m_textures(copy.m_textures){}

const glm::vec4 Material::GetAmbientColor() const
{
	return m_MaterialProperties->Ambient;
}

void Material::SetAmbientColor(const glm::vec4& ambient)
{
	m_MaterialProperties->Ambient = ambient;
}

const glm::vec4 Material::GetDiffuseColor() const
{
	return m_MaterialProperties->Diffuse;
}
void Material::SetDiffuseColor(const glm::vec4& diffuse)
{
	m_MaterialProperties->Diffuse = diffuse;
}

const glm::vec4 Material::GetEmissiveColor() const
{
	return m_MaterialProperties->Emissive;
}
void Material::SetEmissiveColor(const glm::vec4& emissive)
{
	m_MaterialProperties->Emissive = emissive;
}
const glm::vec4 Material::GetSpecularColor() const
{
	return m_MaterialProperties->Specular;
}
void Material::SetSpecularColor(const glm::vec4& specular)
{
	m_MaterialProperties->Specular = specular;
}

float Material::GetSpecularPower() const
{
	return m_MaterialProperties->SpecularPower;
}
void  Material::SetSpecularPower(float specularPower)
{
	m_MaterialProperties->SpecularPower = specularPower;
}

const float Material::GetOpacity() const
{
	return m_MaterialProperties->Opacity;
}
void Material::SetOpacity(float opacity)
{
	m_MaterialProperties->Opacity = opacity;
}

float Material::GetIndexOfRefraction() const
{
	return m_MaterialProperties->IndexOfRefraction;
}
void Material::SetIndexOfRefraction(float indexOfRefraction)
{
	m_MaterialProperties->IndexOfRefraction = indexOfRefraction;
}

float Material::GetClearCoatThickness() const
{
    return m_MaterialProperties->ClearCoatThickness;
}
void Material::SetClearCoatThickness(float clearCoatThickness)
{
    m_MaterialProperties->ClearCoatThickness = clearCoatThickness;
}

float Material::GetRoughness() const
{
    return m_MaterialProperties->Roughness;
}

void Material::SetRoughness(float roughness) 
{
    m_MaterialProperties->Roughness = roughness;
}

float Material::GetMetallic() const
{
    return m_MaterialProperties->Metallic;
}

void Material::SetMetallic(float metallic)
{
    m_MaterialProperties->Metallic = metallic;
}

float Material::GetClearCoatRoughness() const
{
    return m_MaterialProperties->ClearCoatRoughness;
}

void Material::SetClearCoatRoughness(float clearCoatRoughness)
{
    m_MaterialProperties->ClearCoatRoughness = clearCoatRoughness;
}

float Material::GetBumpIntensity() const
{
	return m_MaterialProperties->BumpIntensity;
}
void Material::SetBumpIntensity(float bumpIntensity)
{
	m_MaterialProperties->BumpIntensity = bumpIntensity;
}

void Material::SetTexture(std::shared_ptr<Texture> texture, TextureType type)
{
		m_textures[type] = texture;
        switch (type)
        {
        case TextureType::Ambient:
        {
            m_MaterialProperties->HasAmbientTexture = (texture != nullptr);
        }
        break;
        case TextureType::Emissive:
        {
            m_MaterialProperties->HasEmissiveTexture = (texture != nullptr);
        }
        break;
        case TextureType::Diffuse:
        {
            m_MaterialProperties->HasDiffuseTexture = (texture != nullptr);
        }
        break;
        case TextureType::Specular:
        {
            m_MaterialProperties->HasSpecularTexture = (texture != nullptr);
        }
        break;
        case TextureType::SpecularPower:
        {
            m_MaterialProperties->HasSpecularPowerTexture = (texture != nullptr);
        }
        break;
        case TextureType::Normal:
        {
            m_MaterialProperties->HasNormalTexture = (texture != nullptr);
        }
        break;
        case TextureType::Bump:
        {
            m_MaterialProperties->HasBumpTexture = (texture != nullptr);
        }
        break;
        case TextureType::Opacity:
        {
            m_MaterialProperties->HasOpacityTexture = (texture != nullptr);
        }
        break;
        case TextureType::Albedo:
        {
            m_MaterialProperties->HasAlbedoTexture = (texture != nullptr);
        }
        break;
        case TextureType::Metallic:
        {
            m_MaterialProperties->HasMetallicTexture = (texture != nullptr);
        }
        break;
        case TextureType::Roughness:
        {
            m_MaterialProperties->HasRoughnessTexture = (texture != nullptr);
        }
        break;
        case TextureType::Reflection:
        {
            m_MaterialProperties->HasReflectionTexture = (texture != nullptr);
        }
        break;
        }
}
bool Material::IsTransparent() const
{
	return (m_MaterialProperties->Opacity < 1.0f || m_MaterialProperties->HasOpacityTexture);
}

const MaterialProperties& Material::GetMaterialProperties() const
{
	return *m_MaterialProperties;
}
void Material::SetMaterialProperties(const MaterialProperties& materialProperties)
{
	*m_MaterialProperties = materialProperties;
}