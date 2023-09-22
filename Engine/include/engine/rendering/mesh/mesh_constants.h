#ifndef MESH_CONSTANTS_H
#define MESH_CONSTANTS_H

#include <cstdint> 
#include <string_view>

struct MeshConstants
{
	inline constexpr static const std::uint32_t MAX_LOD_COUNT = 8;
	inline constexpr static const std::uint32_t MAX_STREAM_COUNT = 7;
	inline constexpr static const std::uint32_t MESH_FILE_HEADER_MAGIC_VALUE = 0x123456789;
	inline constexpr static const std::string_view MESH_FILE_EXTENSION = ".gmesh";

};

#endif // MESH_CONSTANTS_H