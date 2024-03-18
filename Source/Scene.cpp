#include <RendererInclude.h>
#include "Scene.h"

namespace Renderer
{
	Scene::Scene() :m_polygons() {}

	void Scene::loadModel(std::string path)
	{
        Polygon m_loadPoly;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        // ========= WIP, unfinished =========

       //  ============================
        m_polygons.push_back(m_loadPoly);
	}


}