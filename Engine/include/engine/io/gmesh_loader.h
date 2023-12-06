#ifndef GMESH_LOADER_H
#define GMESH_LOADER_H

#include <cstdint>
#include <vector>
#include <expected>
#include <filesystem>

struct GMeshDef
{
	//X meshes.data() + vertexFloatBegin
	uint32_t vertexFloatBegin;
	//X PerVertexElement * Vertex Count
	uint32_t vertexFloatCount;
	uint32_t indexBegin;
	uint32_t indexCount;
	std::uint64_t meshFlag;
};


struct GMeshOut
{
	std::vector<uint32_t> indices;
	std::vector<float> vertices;
	std::vector<GMeshDef> gmeshDefs;
};

class IGMeshLoader
{
public:
	virtual ~IGMeshLoader() = default;

	virtual std::expected<GMeshOut,uint32_t> load_mesh_file(std::filesystem::path file) = 0;
private:
};

#endif // GMESH_LOADER_H