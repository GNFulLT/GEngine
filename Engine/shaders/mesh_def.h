#define MAX_LOD_COUNT 8

#extension GL_EXT_shader_explicit_arithmetic_types_int64: enable

struct BoundingBox
{
	vec3 min;
	vec3 max;
};

struct MeshData{
	BoundingBox boundingBox;
	vec4 extents;
	uint lodCount;
	uint indexOffset;
	uint vertexOffset;
	uint vertexCount;
	uint lodOffset[MAX_LOD_COUNT];
	uint64_t meshFlag;
}; 

struct DrawData
{
	uint meshIndex;
	uint materialIndex;
	uint transformIndex;
};

#define MESH_TYPE_HAS_UV 1 << 0 
#define MESH_TYPE_HAS_NORMAL 1 << 1 
#define MESH_TYPE_HAS_TANGENT_BITANGENT 1 << 2

uint calculateVertexElementCount(inout MeshData meshData)
{
	uint vertexElementCount = 3;
	if((meshData.meshFlag & MESH_TYPE_HAS_UV) != 0)
	{
		vertexElementCount += 2;
	}
	if((meshData.meshFlag & MESH_TYPE_HAS_NORMAL) != 0)
	{
		vertexElementCount += 2;
	}
	if((meshData.meshFlag & MESH_TYPE_HAS_TANGENT_BITANGENT) != 0)
	{
		vertexElementCount += 6;
	}
	return vertexElementCount;
}
