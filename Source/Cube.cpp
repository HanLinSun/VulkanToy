#include <Cube.h>
#include <Vulkan/BufferUtils.h>

namespace Renderer
{
	Cube::Cube(Device* device, VkCommandPool commandPool)
	{
        this->m_device = device;
        this->m_commandPool = commandPool;

        std::vector<glm::vec3> pos {glm::vec3(-1, -1, -1),
            glm::vec3(1, -1, -1),
            glm::vec3(1, 1, -1),
            glm::vec3(-1, 1, -1),
            glm::vec3(-1, -1, 1),
            glm::vec3(1, -1, 1),
            glm::vec3(1, 1, 1),
            glm::vec3(-1, 1, 1)};

        std::vector<uint32_t> idx {1, 0, 3, 1, 3, 2,
            4, 5, 6, 4, 6, 7,
            5, 1, 2, 5, 2, 6,
            7, 6, 2, 7, 2, 3,
            0, 4, 7, 0, 7, 3,
            0, 1, 5, 0, 5, 4};

        int count = 36;

        BufferUtils::CreateBufferFromData(this->m_device,this->m_commandPool, this->m_vertices.data(), pos.size() * sizeof(glm::vec3),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_vertexBuffer, m_vertexBufferMemory);

       BufferUtils::CreateBufferFromData(this->m_device, this->m_commandPool, this->m_indices.data(), m_indices.size() * sizeof(uint32_t),
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indexBuffer, m_indexBufferMemory);
	}

    Cube::~Cube()
    {
            vkDestroyBuffer(m_device->GetVkDevice(), m_indexBuffer, nullptr);
            vkFreeMemory(m_device->GetVkDevice(), m_indexBufferMemory, nullptr);

            vkDestroyBuffer(m_device->GetVkDevice(), m_vertexBuffer, nullptr);
            vkFreeMemory(m_device->GetVkDevice(), m_vertexBufferMemory, nullptr);
    }
}