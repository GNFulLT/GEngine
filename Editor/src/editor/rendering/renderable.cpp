#include "internal/rendering/renderable.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/igvulkan_vertex_buffer.h"
Renderable::Renderable(IGVulkanLogicalDevice* device, const std::vector<Vertex_1>& vertices)
{
	m_boundedDevice = device;
	m_vertices = vertices;

}

bool Renderable::init()
{
	auto structSize = sizeof(Vertex_1);
	auto res = m_boundedDevice->create_vertex_buffer(m_vertices.size() * structSize);
	if (!res.has_value())
		return false;

	m_vertexBuffer = res.value();

	//X After creating the vertex buffer push the vertex data

	m_vertexBuffer->copy_data_to_device_memory(m_vertices.data(), m_vertices.size() * structSize);

	m_modelMatrix = m_modelTransform.to_mat4();

	return true;
}

void Renderable::destroy()
{
	m_vertexBuffer->unload();
	delete m_vertexBuffer;
}

gmat4& Renderable::get_model_matrix()
{
	return m_modelMatrix;
}

IGVulkanVertexBuffer* Renderable::get_vertex_buffer()
{
	return m_vertexBuffer;
}
