#include "internal/engine/resource/gmesh_resource.h"
#include <meshoptimizer.h>
#include "engine/rendering/mesh_data.h"

GMeshResource::GMeshResource(IGResourceManager* resManager, std::filesystem::path path, GSharedPtr<IGMeshLoader> meshLoader,bool tryToRemapMesh)
{
	m_path = path;
	m_meshLoader = meshLoader;
	m_resManager = resManager;
	m_tryToRemapMesh = tryToRemapMesh;
}

RESOURCE_INIT_CODE GMeshResource::prepare_impl()
{
	IGMeshLoader* loader = nullptr;
	if(m_meshLoader.is_valid())
	{
		loader = m_meshLoader.get();
	}
	else
	{
		//X Find a resource loader
		loader = m_resManager->select_mesh_loader_by_path(m_path);
		if(loader == nullptr)
			return RESOURCE_INIT_CODE_UNKNOWN_EX;

	}
	auto res = loader->load_mesh_file(m_path);
	if (res.has_value())
	{
		m_loadedMeshes = res.value();
		//X Now create ex datas
		if (m_tryToRemapMesh)
		{
			std::vector<float> newVertices;
			std::vector<uint32_t> newIndices(m_loadedMeshes.indices.size());

			for (auto& mesh : m_loadedMeshes.gmeshDefs)
			{
				std::vector<uint32_t> remapTable(mesh.indexCount);

				auto perVertexElement = calculateVertexElementCount(mesh.meshFlag);

				auto beginOfMeshVertices = m_loadedMeshes.vertices.data() + mesh.vertexFloatBegin;

				std::size_t vertexCount = meshopt_generateVertexRemap(remapTable.data(), nullptr, remapTable.size(),
					beginOfMeshVertices, mesh.vertexFloatCount / perVertexElement, sizeof(float)*perVertexElement);

				auto begin = newVertices.size();
				auto beginIndices = newIndices.size();
				newVertices.resize(newVertices.size() + vertexCount);

				auto destIter = &newVertices[begin];
				meshopt_remapVertexBuffer(destIter, beginOfMeshVertices, vertexCount, sizeof(float) * perVertexElement,remapTable.data());
				meshopt_remapIndexBuffer(&newIndices[mesh.indexBegin], nullptr, mesh.indexCount, remapTable.data());
				
				meshopt_optimizeVertexCache(&newIndices[mesh.indexBegin], &newIndices[mesh.indexBegin], mesh.indexCount, vertexCount);
				meshopt_optimizeOverdraw(&newIndices[mesh.indexBegin], &newIndices[mesh.indexBegin], mesh.indexCount, destIter, vertexCount, sizeof(float) * perVertexElement,1.01f);
				meshopt_optimizeVertexFetch(destIter, &newIndices[mesh.indexBegin], mesh.indexBegin, destIter, vertexCount, sizeof(float) * perVertexElement);
			
				mesh.vertexFloatCount = vertexCount * perVertexElement;
				mesh.vertexFloatBegin = begin;
			}
			m_loadedMeshes.indices.clear();
			m_loadedMeshes.vertices.clear();

			m_loadedMeshes.indices = newIndices;
			m_loadedMeshes.vertices = newVertices;
		}
		
		//X Calculate the bounds ( AABB )

	}
	else
	{
		//X TODO: LOG
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}
}

void GMeshResource::unprepare_impl()
{
	m_loadedMeshes.gmeshDefs.clear();
	m_loadedMeshes.indices.clear();
	m_loadedMeshes.vertices.clear();
}

RESOURCE_INIT_CODE GMeshResource::load_impl()
{
	//X Load to GPU
	return RESOURCE_INIT_CODE_OK;
}

void GMeshResource::unload_impl()
{
}

std::uint64_t GMeshResource::calculateSize() const
{
	return std::uint64_t();
}

std::string_view GMeshResource::get_resource_path() const
{
	return std::string_view();
}

void GMeshResource::destroy_impl()
{
}
