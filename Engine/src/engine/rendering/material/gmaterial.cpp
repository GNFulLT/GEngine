#include "engine/rendering/material/gmaterial.h"
#include <filesystem>
#include <fstream>

void save_texture_files(std::ofstream& ofstream, const std::vector<std::string>& textureFileNames)
{
	uint32_t sz = (uint32_t)textureFileNames.size();
	ofstream.write((char*)&sz, sizeof(uint32_t));
	for (const auto& tex : textureFileNames)
	{
		sz = (uint32_t)tex.length();
		ofstream.write((char*)&sz, sizeof(uint32_t));
		ofstream.write(tex.c_str(),tex.size()*sizeof(char));
	}
}

void load_texture_files(std::ifstream& ifstream,std::vector<std::string>& textureFileNames)
{
	uint32_t sz = 0;
	ifstream.read((char*)&sz, sizeof(uint32_t));
	textureFileNames.resize(sz);
	std::vector<char> inBytes;
	for (auto& s : textureFileNames)
	{
		uint32_t sz = 0;
		ifstream.read((char*)&sz, sizeof(uint32_t));
		//X Null terminated add 0 to end of the string
		inBytes.resize(sz + 1);
		ifstream.read(inBytes.data(), sz + 1);
		s = std::string(inBytes.data());
	}
}
bool MaterialDescription::save_materials(const std::vector<MaterialDescription>& materials, const std::vector<std::string>& textureFileNames, const char* filePath)
{
	std::string pth(filePath);
	pth += MaterialDescription::EXTENSION_NAME;
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);
	ofstream.exceptions(std::ifstream::badbit);
	bool failed = false;
	try
	{
		uint32_t sz = (uint32_t)materials.size();
		ofstream.write((char*)&sz,sizeof(uint32_t));
		ofstream.write((char*)materials.data(),sizeof(uint32_t)* materials.size());

		save_texture_files(ofstream,textureFileNames);
	}
	catch (const std::ifstream::failure& e)
	{
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}
	ofstream.close();
	return failed;
}

ENGINE_API MATERIAL_DECODE_RESULT MaterialDescription::load_materials(const char* filePath, std::vector<MaterialDescription>& materials, std::vector<std::string>& textureFileNames)
{
	std::filesystem::path path(filePath);
	if (!std::filesystem::exists(path))
	{
		return MATERIAL_DECODE_RESULT_UNKNOWN_ERROR;
	}
	if (std::filesystem::is_directory(path) || strcmp(path.extension().string().c_str(), MaterialDescription::EXTENSION_NAME.data()) != 0)
	{
		return MATERIAL_DECODE_RESULT_UNKNOWN_ERROR;
	}

	std::ifstream ifstream(path, std::ios::in | std::ios::binary);

	//X TODO : GDNEWDA

	MATERIAL_DECODE_RESULT err = MATERIAL_DECODE_RESULT_OK;

	ifstream.exceptions(std::ifstream::badbit);

	try
	{
		uint32_t sz = 0;
		ifstream.read((char*)&sz, sizeof(sz));
		materials.resize(sz);
		ifstream.read((char*)materials.data(),materials.size()*sizeof(MaterialDescription));
		load_texture_files(ifstream, textureFileNames);
	}
	catch (const std::ifstream::failure& e)
	{
		err = MATERIAL_DECODE_RESULT_UNKNOWN_ERROR;
	}
	catch (const std::exception& e)
	{
		err = MATERIAL_DECODE_RESULT_UNKNOWN_ERROR;
	}

	ifstream.close();

	return err;

}
