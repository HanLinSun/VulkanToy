#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"

namespace Renderer
{
	Scene::Scene(){}
	Scene::Scene(std::shared_ptr<Camera> cam):m_Camera(cam){}

	Scene::~Scene()
	{
		std::cout << "Scene deconstructor called" << std::endl;
	}

	const ModelGroup* Scene::GetSceneModelGroup(int idx)
	{
		return m_modelGroups[idx].get();
	}

	void Scene::DestroyVKResources()
	{
		for (auto& modelgroup : m_modelGroups)
		{
			modelgroup->DestroyVKResources();
		}
		for (auto& texture : m_textures)
		{
			texture->DestroyVKResources();
		}
		for (auto& material : m_materials)
		{
			material->DestroyVKResources();
		}
	}

	void Scene::AddTexture(std::shared_ptr<Texture2D> texture)
	{
		m_textures.push_back(texture);
	}

	void Scene::AddLight(Light light)
	{
		m_lights.push_back(light);
	}

	std::vector<Light> Scene::GetLights()
	{
		return m_lights;
	}

	int Scene::GetLightSize() const
	{
		return m_lights.size();
	}

	std::shared_ptr<Texture2D> Scene::GetTexture(int idx)
	{
		return m_textures[idx];
	}

	std::vector <std::shared_ptr<Texture2D>> Scene::GetTextures()
	{
		return m_textures;
	}

	void Scene::AddMaterial(std::shared_ptr<Material> material)
	{
		m_materials.push_back(material);
	}

	int Scene::GetCurrentTextureSize() const
	{
		int size = m_textures.size();
		return size;
	}


	std::shared_ptr<Material> Scene::GetMaterial(int idx)
	{
		return m_materials[idx];
	}

	int Scene::GetMaterialSize() const
	{
		return m_materials.size();
	}

	std::vector<PBRMaterialData> Scene::GeneratePBRMaterialData()
	{
		std::vector<PBRMaterialData> m_pbrMats;
		for (auto& mat : m_materials)
		{
			PBRMaterialData pbrMat;
			pbrMat.baseColor = mat->GetDiffuseColor();
			pbrMat.emission = mat->GetEmissiveColor();
			pbrMat.roughness = mat->GetRoughness();
			pbrMat.metallic = mat->GetMetallic();
			pbrMat.sheen = mat->GetSheen();
			pbrMat.sheenTint = mat->GetSheenTint();
			pbrMat.anisotropic = mat->GetAnisotropic();

			pbrMat.clearcoatThickness = mat->GetClearCoatThickness();
			pbrMat.clearcoatRoughness = mat->GetClearCoatRoughness();
			pbrMat.transmission = mat->GetTransmission();

			pbrMat.specTrans = mat->GetSpecTrans();
			pbrMat.specularTint = mat->GetSpecularTint();
			pbrMat.subsurface = mat->GetSubSurface();
			pbrMat.ior = mat->GetIndexOfRefraction();

			pbrMat.albedoTextureID = mat->GetTextureID(TextureType::Diffuse);
			pbrMat.normalTextureID = mat->GetTextureID(TextureType::Normal);
			pbrMat.metallicRoughnessTextureID = mat->GetTextureID(TextureType::MetallicRoughness);
			pbrMat.emissonMapTextureID = mat->GetTextureID(TextureType::Emissive);
			m_pbrMats.push_back(pbrMat);
		}
		return m_pbrMats;
	}

	std::vector<Sphere> Scene::GetSpheres()
	{
		return m_spheres;
	}

	std::vector<std::shared_ptr<Material>> Scene::GetMaterials()
	{
		return m_materials;
	}

	void Scene::SetSceneCamera(std::shared_ptr<Camera> Camera)
	{
		m_Camera = Camera;
	}

	const std::vector<ModelGroup*> Scene::GetSceneModelGroupsRaw()
	{
		std::vector<ModelGroup*> rawPointers;
		for (const auto& modelGroup : m_modelGroups) {
			rawPointers.push_back(modelGroup.get()); // Get raw pointer
		}
		return rawPointers; // Return the vector of raw pointers
	}

	const std::shared_ptr<Camera> Scene::GetCamera()
	{
		return m_Camera;
	}

	size_t Scene::GetModelGroupSize()
	{
		return m_modelGroups.size();
	}

	void Scene::AddModelGroup(std::unique_ptr<ModelGroup> modelgroup)
	{
		m_modelGroups.push_back(std::move(modelgroup));
	}

	std::vector<Triangle> Scene::GetTriangles()
	{
		return m_triangles;
	}

	void Scene::AddSphere(Sphere sphere)
	{
		m_spheres.push_back(sphere);
	}

	int Scene::GetSphereSize() const
	{
		return m_spheres.size();
	}

	void Scene::AddTriangle(Triangle& triangle)
	{
		m_triangles.push_back(triangle);
	}
}