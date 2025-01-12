#pragma once
#include <RendererInclude.h>
#include <SceneStructs.h>
#include <Camera.h>
#include <CameraController.h>
#include <Model.h>
#include <BVH.h>
#include <GPUModel.h>
#include <ModelFileLoader.h>
#include <fstream>

namespace Renderer
{
	class Scene
	{
	public:

		Scene(std::shared_ptr<Camera> Camera);
		Scene();
		~Scene();

		const std::vector<ModelGroup*> GetSceneModelGroupsRaw();

		void AddModelGroup(std::unique_ptr<ModelGroup> modelgroup);
		const std::shared_ptr<Camera> GetCamera();
		void SetSceneCamera(std::shared_ptr<Camera> Camera);
		void DestroyVKResources();

		size_t GetModelGroupSize();
		const ModelGroup* GetSceneModelGroup(int idx);

		void AddTexture(std::shared_ptr<Texture2D> texture);
		std::shared_ptr<Texture2D> GetTexture(int idx);
		std::vector <std::shared_ptr<Texture2D>> GetTextures();
		int GetCurrentTextureSize() const;

		void AddMaterial(std::shared_ptr<Material> material);
		std::shared_ptr<Material> GetMaterial(int idx);
		std::vector<std::shared_ptr<Material>> GetMaterials();
		int GetMaterialSize() const;

		std::vector<Triangle> GetTriangles();
		void AddTriangle(Triangle& triangle);

		void GetTriangleFromModelGroups();

		void AddSphere(Sphere sphere);
		std::vector<Sphere> GetSpheres();
		int GetSphereSize() const;

		void AddLight(LightGPU light);
		std::vector<LightGPU> GetLights();
		int GetLightSize() const;

		std::vector<PBRMaterialData> GeneratePBRMaterialData();

		void InitTestSpheresScene_1();

		void GenerateBVHObjectArray();
		std::vector<BVHObject> GetBVHObjectArray();

		std::ifstream fp_in;

	private:

		std::vector<std::unique_ptr<ModelGroup>> m_modelGroups;
		std::vector<std::shared_ptr<Texture2D>> m_textures;
		std::vector<std::shared_ptr<Material>> m_materials;
		std::vector<BVHObject> m_bvhObjects;

		std::vector<std::shared_ptr<TestMaterial>> m_testMaterials;
		
		std::vector<Sphere> m_spheres;
		std::vector<Triangle> m_triangles;
		std::vector<LightGPU> m_lights;

		std::shared_ptr<Camera> m_Camera;
		std::unique_ptr<TextureCubeMap> m_skyboxTexture;

	};

}