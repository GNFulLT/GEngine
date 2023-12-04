#define MAX_LOD_COUNT 8

#extension GL_EXT_shader_explicit_arithmetic_types_int64: enable
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require

struct GMeshlet
{
	uint vertexOffset;
	uint triangleOffset;
	uint vertexCount;
	uint triangleCount;

	float center[3];
	float radius;
	uint8_t coneAxis[3];
	uint8_t coneCutoff;
};

struct MeshletDescriptor{
	uint meshletOffset;
	uint meshletVerticesOffset;
	uint meshletTrianglesOffset;

	uint meshletCount;
	uint meshletVerticesCount;
	uint meshletTrianglesCount;
}; 

const int kShaderGroupSizeNV = 32;
const int kMaxVerticesPerMeshlet = 64;
const int kMaxTrianglesPerMeshlet = 124;