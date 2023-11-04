#ifndef GMESH_ENCODER_H
#define GMESH_ENCODER_H

#include "engine/io/iowning_glogger.h"
#include "engine/rendering/mesh/gmesh.h"
#include <expected>
#include <filesystem>
#include <vector>
#include "engine/rendering/scene/scene_config.h"
#include "engine/rendering/mesh_data.h"
#include <assimp/mesh.h>

enum GMESH_ENCODER_ERROR
{
	GMESH_ENCODER_ERROR_UNKNOWN
};

enum GMESH_ENCODER_SAVE_ERROR
{
	GMESH_ENCODER_SAVE_ERROR_UNKNOWN
};

//X Not thread safe so call this in one thread per object
class GMeshEncoder
{
public:
	GMeshEncoder(IOwningGLogger* logger);
	std::expected<int, GMESH_ENCODER_ERROR> decode_and_encode_to_gmesh(std::filesystem::path path);
	std::expected<int, GMESH_ENCODER_SAVE_ERROR> save_to_file_and_reset(const char* path);


	static std::expected<int, GMESH_ENCODER_SAVE_ERROR> save_to_file_and_reset(const char* path, const MeshData& m);

	static GMesh ai_mesh_to_gmesh(const aiMesh* aiMesh,bool loadLODS,int scale);
	static GMesh ai_mesh_to_gmesh(const aiMesh* aiMesh, bool loadLODS, int scale, MeshData& mesh,uint32_t& vertexOffset,uint32_t& indexOffset);
	static GMeshMeshlet ai_mesh_to_gmesh_meshlet(const aiMesh* aiMesh, bool loadLODS, int scale, GMeshletData& meshlet,uint32_t& vertexOffset, uint32_t& indexOffset,uint32_t& meshletOffset,uint32_t& meshletVertexOffset,uint32_t& meshletTriagleOffset);

	static void process_lods(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods, uint32_t elementPerVertex);
	static void process_meshlets(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<GMeshlet>& outMeshlets,std::vector<uint32_t>& outMeshletVert,std::vector<uint8_t>& outMeshletTriangles,uint32_t elementPerVertex,uint32_t meshletVertexOffset,uint32_t meshletTriangleOffset);


	void reset();
private:
	bool verbose = true;

	IOwningGLogger* m_logger;

private:
	inline static uint32_t g_indexOffset = 0;
	inline static uint32_t g_vertexOffset = 0;

	inline static const uint32_t g_numElementsToStore = 3 + 2 + 2;
	inline static MeshData g_MeshData;
};


#endif // GMESH_ENCODER_H