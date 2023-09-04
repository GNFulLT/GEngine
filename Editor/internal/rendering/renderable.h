#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "internal/rendering/vertex_types.h"
#include "public/math/gmat4.h"
#include "public/math/gtransform.h"

#include <vector>

class IGVulkanVertexBuffer;
class IGVulkanLogicalDevice;

class Renderable
{
public:
	Renderable(IGVulkanLogicalDevice* device, const std::vector<Vertex_1>& vertices);

	bool init();

	void destroy();

	gmat4& get_model_matrix();

	IGVulkanVertexBuffer* get_vertex_buffer();
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	std::vector<Vertex_1> m_vertices;
	IGVulkanVertexBuffer* m_vertexBuffer;

	gtransform m_modelTransform;
	gmat4 m_modelMatrix;
};

#endif // RENDERABLE_H