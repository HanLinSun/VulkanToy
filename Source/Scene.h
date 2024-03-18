#pragma once
#include <RendererInclude.h>
#include "Camera.h"

namespace Renderer
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoords;
	};
	struct Triangle
	{
		std::array<Vertex, 3> verts;
	};

	struct Polygon
	{
		std::vector<Triangle> triangles;
	};

	class Scene
	{
	public:

		Scene();
		~Scene();

		void loadModel(std::string path);
		const std::vector<Polygon> getSceneObjects();


	private:
		std::vector<Polygon> m_polygons;
		Camera m_camera;
	};
}