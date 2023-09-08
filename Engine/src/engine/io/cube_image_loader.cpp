#include "volk.h"
#include "internal/engine/io/cube_image_loader.h"
#include "public/core/stb_image.h"
#include <vector>
#include "internal/engine/io/cubemap_utils.h"

static void float24to32(int w, int h, const float* img24, float* img32)
{
	const int numPixels = w * h;
	for (int i = 0; i != numPixels; i++)
	{
		*img32++ = *img24++;
		*img32++ = *img24++;
		*img32++ = *img24++;
		*img32++ = 1.0f;
	}
}
bool CubemapImageLoader::inject_create_image_info(VkImageCreateInfo* info, VkImageViewCreateInfo* ivInfo)
{
	info->arrayLayers = 6;
	ivInfo->format = VK_FORMAT_R32G32B32A32_SFLOAT;
	ivInfo->viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ivInfo->subresourceRange.layerCount = 6;
	return true;
}
const std::vector<GIMAGETYPE>* CubemapImageLoader::get_supported_image_types()
{
	return &m_supportedImageTypes;
}
CubemapImageLoader::CubemapImageLoader()
{
	m_loaderName = "cubemap_loader";
	m_supportedImageTypes.push_back(GIMAGETYPE_CUBEMAP);
}

std::string_view CubemapImageLoader::get_loader_name()
{
	return m_loaderName;
}

GImage_Descriptor* CubemapImageLoader::load(std::string_view path, int dedicatedFormat)
{
	int texWidth, texHeight, texChannels;
	auto* pixels = stbi_loadf(path.data(), &texWidth, &texHeight, &texChannels, 3);
	assert(pixels != nullptr);
	//X Erase the A float value
	std::vector<float> img32(texWidth * texHeight * 4);

	float24to32(texWidth, texHeight, pixels, img32.data());

	void* imageData;
	GImage_Descriptor* img = new GImage_Descriptor();

	stbi_image_free((void*)pixels);
	{


		Bitmap in(texWidth, texHeight, 4, eBitmapFormat_Float, img32.data());
		Bitmap out = convertEquirectangularMapToVerticalCross(in);

		Bitmap cube = convertVerticalCrossToCubeMapFaces(out);
		imageData = malloc(cube.data_.size());
		memcpy(imageData, &cube.data_[0], cube.data_.size());	
		img->size = cube.data_.size();
		img->width = cube.w_;
		img->height = cube.h_;

	}

	img->channelCount = texChannels;
	
	img->format = VK_FORMAT_R32G32B32A32_SFLOAT;
	img->pixels = (uint8_t*)imageData;
	img->imageType = GIMAGETYPE_2D;
	img->flag = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	return img;
}

void CubemapImageLoader::unload(GImage_Descriptor* img)
{
	stbi_image_free(img->pixels);
	delete img;
}
