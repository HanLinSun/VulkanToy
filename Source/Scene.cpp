#include "Scene.h"
//This is equal to include real implements
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Headers/modelFileLoader.h"
#include <glm/gtc/matrix_inverse.hpp>
namespace Renderer
{
	Scene::Scene(){}
	Scene::Scene(std::shared_ptr<Camera> cam):m_Camera(cam){}

	Scene::~Scene()
	{
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

	std::vector<SphereCPU> Scene::GetCPUSpheres()
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

	void Scene::GetTriangleFromModelGroups()
	{
		m_triangles.resize(0);
		for (auto& modelgroup : m_modelGroups)
		{
			auto modelGroupTriangles = modelgroup->GetAllModelTriangles();
			m_triangles.insert(m_triangles.end(), modelGroupTriangles.begin(), modelGroupTriangles.end());
		}
	}

	void Scene::AddSphere(SphereCPU sphere)
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

	std::vector<SphereGPU>  Scene::GetGPUSpheres()
	{
		std::vector<SphereGPU> sphereGPU;
		for (auto& sphere : m_spheres)
		{
			sphereGPU.push_back(sphere.GetGPUSphere());
		}
		return sphereGPU;
	}

	// =========== For Debug Only ===============
	
	void Scene::InitTestSpheresScene_1()
	{
		SphereCPU test_sphere1;
		test_sphere1.position = glm::vec3(0, 1, -1);
		test_sphere1.radius = 0.4;
		test_sphere1.translate = glm::vec3(0, 0, 0);
		test_sphere1.rotate = glm::vec3(0, 0, 0);
		test_sphere1.scale = glm::vec3(1.5, 1.5, 1.5);

		SphereCPU test_sphere2;
		test_sphere2.position = glm::vec3(0, -10, -1);
		test_sphere2.radius = 10;
		test_sphere2.translate = glm::vec3(0, 0, 0);
		test_sphere2.rotate = glm::vec3(0, 0, 0);
		test_sphere2.scale = glm::vec3(1, 1, 1);

		AddSphere(test_sphere1);
		AddSphere(test_sphere2);
	}
	// =====================================
	void Scene::GenerateMeshArray()
	{
		int startTriangleIdx = 0;
		for (auto& modelgroup : m_modelGroups)
		{
			for (int i = 0; i < modelgroup->GetModelSize(); i++)
			{
				auto model = modelgroup->GetModelAt(i);
				Mesh mesh;
				mesh.meshType = MESH;
				mesh.sphereIdx = -1;
				mesh.material_ID = model->material_ID;
				mesh.transformMatrix = model->GetTransformMatrix();
				mesh.inverseTransform = glm::inverse(mesh.transformMatrix);
				mesh.inverseTranspose = glm::inverseTranspose(mesh.transformMatrix);
				mesh.startTriangleIdx = startTriangleIdx;
				mesh.triangleNums = model->GetTriangleSize();
				startTriangleIdx += model->GetTriangleSize();
				m_meshes.push_back(mesh);
			}
		}

		for (int i=0;i<m_spheres.size();i++)
		{
			m_meshes.push_back(m_spheres[i].BuildMesh());
		}
	}

	std::vector<Mesh> Scene::GetMeshArray()
	{
		if (m_meshes.size() == 0)
		{
			GenerateMeshArray();
		}
		return m_meshes;
	}
	
	std::vector<BVHObject> Scene::GetBVHObjectArray()
	{
		if (m_bvhObjects.size() == 0)
		{
			GenerateBVHObjectArray();
		}
		return m_bvhObjects;
	}

    void Scene::GenerateBVHObjectArray()
	{
		int triangleIdx = 0;
		int sphereIdx = 0;
		for (auto triangle : m_triangles)
		{
			BVHObject object;
			object.triangle_index = triangleIdx;
			object.sphere_index = -1;
			object.boundbox = BVHBuildTool::CreateBoundingBox(triangle);
			object.ComputeCentroid();
			triangleIdx++;
			m_bvhObjects.push_back(object);
		}

		for(auto sphere:m_spheres)
		{
			BVHObject object;
			object.sphere_index = sphereIdx;
			object.triangle_index = -1;
			object.boundbox = BVHBuildTool::CreateBoundingBox(sphere.GetGPUSphere());
			object.ComputeCentroid();
			sphereIdx++;
			m_bvhObjects.push_back(object);
		}
	}

}