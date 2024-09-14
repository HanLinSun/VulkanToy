#include "Model.h"
#include "Vulkan/BufferUtils.h"
namespace Renderer
{
	Model::Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		:m_device(device),m_vertices(vertices),m_indices(indices)
	{
		if (m_vertices.size() > 0)
		{
			BufferUtils::CreateBufferFromData(device, commandPool, this->m_vertices.data(), m_vertices.size() * sizeof(Vertex),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_vertexBuffer, m_vertexBufferMemory);
		}

		if (m_indices.size() > 0)
		{
			BufferUtils::CreateBufferFromData(device, commandPool, this->m_indices.data(), m_indices.size() * sizeof(uint32_t),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indexBuffer, m_indexBufferMemory);
		}
		m_modelBufferObject.modelMatrix = glm::mat4(1.0f);
		BufferUtils::CreateBufferFromData(device, commandPool, &m_modelBufferObject, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_modelUniformBuffer, m_modelUniformBufferMemory);

	}


	const std::vector<Vertex>& Model::GetVertices() const {
		return m_vertices;
	}

	VkBuffer Model::GetVertexBuffer() const {
		return m_vertexBuffer;
	}

	const std::vector<uint32_t>& Model::GetIndices() const {
		return m_indices;
	}

	VkBuffer Model::GetIndexBuffer() const {
		return m_indexBuffer;
	}

	const ModelBufferObject& Model::GetModelBufferObject() const {
		return m_modelBufferObject;
	}

	VkBuffer Model::GetModelUniformBuffer() const {
		return m_modelUniformBuffer;
	}

	void Model::SetTexture(Texture _texture, TextureType type)
	{

		//this->texture = texture;
		//this->textureView = Image::CreateView(device, texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		// --- Specify all filters and transformations ---
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		// Interpolation of texels that are magnified or minified
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// Addressing mode
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		// Anisotropic filtering
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;

		// Border color
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// Choose coordinate system for addressing texels --> [0, 1) here
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		// Comparison function used for filtering operations
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		// Mipmapping
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &_texture.sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler");
		}

		this->m_textures[type] = _texture;
	}

 }
