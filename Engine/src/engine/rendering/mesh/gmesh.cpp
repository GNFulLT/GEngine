#include "engine/rendering/mesh/gmesh.h"
#include "engine/rendering/mesh/gmesh_file.h"
#include <stdio.h>
#include <filesystem>
#include <fstream>
#include "engine/rendering/mesh_data.h"

std::expected<MeshData*, GMESH_DECODE_ERROR> decode_file(const char* filePath)
{
	std::filesystem::path path(filePath);
	if (!std::filesystem::exists(path))
	{
		return std::unexpected(GMESH_DECODE_ERROR_FILE_NOT_EXIST);
	}
	if (std::filesystem::is_directory(path) || strcmp(path.extension().string().c_str(), MeshConstants::MESH_FILE_EXTENSION.data()) != 0)
	{
		return std::unexpected(GMESH_DECODE_ERROR_FILE_FORMAT_UNKNOWN);
	}
	
	std::ifstream ifstream(path, std::ios::in | std::ios::binary);
	
	//X TODO : GDNEWDA

	MeshData* meshFile = new MeshData();
	GMESH_DECODE_ERROR err = GMESH_DECODE_ERROR_UNKNOWN;
	bool failed = false;
	
	ifstream.exceptions(std::ifstream::badbit);
	
	try
	{
		GMeshFileHeader header;
		ifstream.read((char*)&header, sizeof(GMeshFileHeader));

		if (header.magicValue != MeshConstants::MESH_FILE_HEADER_MAGIC_VALUE)
		{
			failed = true;
			err = GMESH_DECODE_ERROR_FILE_CORRUPTED;
		}
		else
		{
			meshFile->meshes_.resize(header.meshCount);
			meshFile->boxes_.resize(header.meshCount);

			ifstream.read((char*)meshFile->meshes_.data(), sizeof(GMesh)*header.meshCount);
			ifstream.read((char*)meshFile->boxes_.data(),sizeof(BoundingBox)*header.meshCount);

			meshFile->indexData_.resize(header.indexDataSize/sizeof(uint32_t));
			meshFile->vertexData_.resize(header.vertexDataSize/sizeof(float));

			ifstream.read((char*)meshFile->indexData_.data(), header.indexDataSize);
			ifstream.read((char*)meshFile->vertexData_.data(),  header.vertexDataSize);

		}


	}
	catch (const std::ifstream::failure& e)
	{
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}
	
	ifstream.close();

	if (failed)
	{
		delete meshFile;
		return std::unexpected(err);
	}

	return meshFile;

}
