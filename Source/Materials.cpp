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

Material::Material(const Material& copy):m_MaterialProperties(NewMaterialProperties(*copy.m_MaterialProperties), &DeleteMaterialProperties){}

Material::Material(const Material* copy) :m_MaterialProperties(NewMaterialProperties(*copy->m_MaterialProperties), &DeleteMaterialProperties) {}

const glm::vec4 Material::GetAmbientColor() const
{
	return m_MaterialProperties->Ambient;
}

void Material::DestroyVKResources()
{
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

glm::float32_t Material::GetSpecularPower() const
{
	return m_MaterialProperties->SpecularPower;
}
void Material::SetSpecularPower(glm::float32_t specularPower)
{
	m_MaterialProperties->SpecularPower = specularPower;
}

glm::float32_t  Material::GetSheenTint() const
{
    return m_MaterialProperties->SheenTint;
}

void Material::SetSheenTint(glm::float32_t sheenTint)
{
    m_MaterialProperties->SheenTint = sheenTint;
}

glm::float32_t  Material::GetSpecularTint() const
{
    return m_MaterialProperties->SpecularTint;
}

void  Material::SetSpecularTint(glm::float32_t specularTint)
{
    m_MaterialProperties->SpecularTint = specularTint;
}

glm::float32_t  Material::GetSpecTrans() const
{
    return m_MaterialProperties->SpecTrans;
}

void  Material::SetSpecTrans(glm::float32_t specTrans)
{
    m_MaterialProperties->SpecTrans = specTrans;
}

glm::float32_t Material::GetSheen() const
{
    return m_MaterialProperties->Sheen;
}

void Material::SetSheen(glm::float32_t sheen)
{
    m_MaterialProperties->Sheen = sheen;
}

glm::float32_t  Material::GetAnisotropic() const
{
    return m_MaterialProperties->Anisotropic;
}

void  Material::SetAnisotropic(glm::float32_t anisotropic)
{
    m_MaterialProperties->Anisotropic = anisotropic;
}

const glm::float32_t Material::GetOpacity() const
{
	return m_MaterialProperties->Opacity;
}
void Material::SetOpacity(glm::float32_t opacity)
{
	m_MaterialProperties->Opacity = opacity;
}

glm::float32_t Material::GetIndexOfRefraction() const
{
	return m_MaterialProperties->IndexOfRefraction;
}
void Material::SetIndexOfRefraction(glm::float32_t indexOfRefraction)
{
	m_MaterialProperties->IndexOfRefraction = indexOfRefraction;
}

glm::float32_t Material::GetClearCoatThickness() const
{
    return m_MaterialProperties->ClearCoatThickness;
}
void Material::SetClearCoatThickness(glm::float32_t clearCoatThickness)
{
    m_MaterialProperties->ClearCoatThickness = clearCoatThickness;
}

glm::float32_t Material::GetSubSurface() const
{
    return m_MaterialProperties->SubSurface;
}

void Material::SetSubSurface(glm::float32_t subsurface)
{
    m_MaterialProperties->SubSurface = subsurface;
}

glm::float32_t Material::GetRoughness() const
{
    return m_MaterialProperties->Roughness;
}
void Material::SetRoughness(glm::float32_t roughness)
{
    m_MaterialProperties->Roughness = roughness;
}

glm::float32_t Material::GetMetallic() const
{
    return m_MaterialProperties->Metallic;
}
void Material::SetMetallic(glm::float32_t metallic)
{
    m_MaterialProperties->Metallic = metallic;
}

glm::float32_t Material::GetClearCoatRoughness() const
{
    return m_MaterialProperties->ClearCoatRoughness;
}
void Material::SetClearCoatRoughness(glm::float32_t clearCoatRoughness)
{
    m_MaterialProperties->ClearCoatRoughness = clearCoatRoughness;
}

glm::float32_t Material::GetBumpIntensity() const
{
	return m_MaterialProperties->BumpIntensity;
}
void Material::SetBumpIntensity(glm::float32_t bumpIntensity)
{
	m_MaterialProperties->BumpIntensity = bumpIntensity;
}

void Material::SetTexture(int textureID, TextureType type)
{
        switch (type)
        {
        case TextureType::Ambient:
        {
            m_MaterialProperties->AmbientTextureIdx = textureID;
        }
        break;
        case TextureType::Emissive:
        {
            m_MaterialProperties->EmissiveTextureIdx = textureID;
        }
        break;
        case TextureType::Diffuse:
        {
            m_MaterialProperties->DiffuseTextureIdx = textureID;
        }
        break;
        case TextureType::Specular:
        {
            m_MaterialProperties->SpecularTextureIdx = textureID;
        }
        break;
        case TextureType::SpecularPower:
        {
            m_MaterialProperties->SpecularPowerTextureIdx = textureID;
        }
        break;
        case TextureType::Normal:
        {
            m_MaterialProperties->NormalTextureIdx = textureID;
        }
        break;
        case TextureType::Bump:
        {
            m_MaterialProperties->BumpTextureIdx = textureID;
        }
        break;
        case TextureType::Opacity:
        {
            m_MaterialProperties->OpacityTextureIdx = textureID;
        }
        break;
        case TextureType::Albedo:
        {
            m_MaterialProperties->AlbedoTextureIdx = textureID;
        }
        break;
        case TextureType::MetallicRoughness:
        {
            m_MaterialProperties->MetallicRoughnessTextureIdx = textureID;
        }
        break;
        case TextureType::Reflection:
        {
            m_MaterialProperties->ReflectionTextureIdx = textureID;
        }
        break;
        }
}

int Material::GetTextureID(TextureType type)
{
    switch (type)
    {
    case TextureType::Ambient:
    {
       return m_MaterialProperties->AmbientTextureIdx;
    }
    break;
    case TextureType::Emissive:
    {
        return m_MaterialProperties->EmissiveTextureIdx;
    }
    break;
    case TextureType::Diffuse:
    {
        return m_MaterialProperties->DiffuseTextureIdx;
    }
    break;
    case TextureType::Specular:
    {
        return m_MaterialProperties->SpecularTextureIdx;
    }
    break;
    case TextureType::SpecularPower:
    {
        return m_MaterialProperties->SpecularPowerTextureIdx;
    }
    break;
    case TextureType::Normal:
    {
        return m_MaterialProperties->NormalTextureIdx;
    }
    break;
    case TextureType::Bump:
    {
        return m_MaterialProperties->BumpTextureIdx;
    }
    break;
    case TextureType::Opacity:
    {
        return m_MaterialProperties->OpacityTextureIdx;
    }
    break;
    case TextureType::Albedo:
    {
        return m_MaterialProperties->AlbedoTextureIdx;
    }
    break;
    case TextureType::MetallicRoughness:
    {
        return m_MaterialProperties->MetallicRoughnessTextureIdx;
    }
    break;
    case TextureType::Reflection:
    {
        return m_MaterialProperties->ReflectionTextureIdx;
    }
    break;
    }
    return -1;
}

glm::vec4 Material::GetTransmittance() const
{
    return m_MaterialProperties->Transmittance;
}

void Material::SetTransmittance(glm::vec4 transmittance)
{
    m_MaterialProperties->Transmittance = transmittance;
}

glm::float32_t Material::GetTransmission() const
{
    return m_MaterialProperties->Transmission;
}

void Material::SetTransmission(glm::float32_t transmission)
{
    m_MaterialProperties->Transmission = transmission;
}

bool Material::IsTransparent() const
{
	return (m_MaterialProperties->Opacity < 1.0f ||( m_MaterialProperties->OpacityTextureIdx!=-1));
}

const MaterialProperties& Material::GetMaterialProperties() const
{
	return *m_MaterialProperties;
}
void Material::SetMaterialProperties(const MaterialProperties& materialProperties)
{
	*m_MaterialProperties = materialProperties;
}

