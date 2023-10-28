#include "engine/rendering/mesh_data.h"
#include <glm/glm.hpp>

void recalculateBoundingBoxes(MeshData& m)
{
	m.boxes_.clear();

	for (const auto& mesh : m.meshes_)
	{
		uint32_t elementPerVertex = calculateVertexElementCount(mesh.meshFlag);
		
		const auto numIndices = mesh.getLODIndicesCount(0);

		glm::vec3 vmin(std::numeric_limits<float>::max());
		glm::vec3 vmax(std::numeric_limits<float>::lowest());

		for (auto i = 0; i != numIndices; i++)
		{
			auto vtxOffset = (elementPerVertex*m.indexData_[mesh.indexOffset + i]) + mesh.vertexOffset;
			const float* vf = &m.vertexData_[vtxOffset];
			vmin = glm::min(vmin, glm::vec3(vf[0], vf[1], vf[2]));
			vmax = glm::max(vmax, glm::vec3(vf[0], vf[1], vf[2]));
		}

		m.boxes_.emplace_back(vmin, vmax);
	}
}

uint32_t calculateVertexElementCount(uint64_t meshFlag)
{
	uint32_t elementPerVertex = 3;
	if ((meshFlag & GMESH_COMPONENT_HAS_UV) != 0)
	{
		elementPerVertex += 2;
	}
	if ((meshFlag & GMESH_COMPONENT_HAS_NORMAL) != 0)
	{
		elementPerVertex += 2;
	}
	if ((meshFlag & GMESH_COMPONENT_HAS_TANGENTS_BITANGENTS) != 0)
	{
		elementPerVertex += 6;
	}
	return elementPerVertex;
}
