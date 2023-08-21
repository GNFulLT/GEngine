#ifndef GTEXTURE_RESOURCE_H
#define GTEXTURE_RESOURCE_H

#include "engine/resource/igtexture_resource.h"
#include "engine/io/gimage_descriptor.h"

class IGVulkanLogicalDevice;
class IVulkanImage;

class GTextureResource : public IGTextureResource
{
public:
	GTextureResource(std::string_view filePath,IImageLoader* loader, IGVulkanLogicalDevice* parentDevice);
	// If system is ready for load operation. This method will be called and ask the resource if there is any internal things prepare them and if u are ready too
	// return true and load implementation will be started

	virtual RESOURCE_INIT_CODE prepare_impl() override;

	// Undo method for preparing. Will be called after unload.

	virtual void unprepare_impl() override;

	// Loads the resource

	virtual RESOURCE_INIT_CODE load_impl() override;

	// Unloads the resource

	virtual void unload_impl() override;

	// Will be called after load operation
	virtual std::uint64_t calculateSize() const override;

	virtual std::string_view get_resource_path() const override;

private:
	std::string m_filePath;
	IGVulkanLogicalDevice* m_boundedDevice;
	GImage_Descriptor* m_imageDescriptor;
	bool m_imageIsInCPU;
	IVulkanImage* m_gpuBuffer;
};

#endif // GTEXTURE_RESOURCE_H