#ifndef GMESH_RESOURCE_H
#define GMESH_RESOURCE_H

#include "engine/resource/iresource.h"
#include "engine/io/gmesh_loader.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/manager/igresource_manager.h"
class GMeshResource : public IResource
{
public:
	GMeshResource(IGResourceManager* resManager, std::filesystem::path path, GSharedPtr<IGMeshLoader> meshLoader,bool tryToRemapMesh);
	
	virtual RESOURCE_INIT_CODE prepare_impl() override;
	virtual void unprepare_impl() override;
	virtual RESOURCE_INIT_CODE load_impl() override;
	virtual void unload_impl() override;
	virtual std::uint64_t calculateSize() const override;
	virtual std::string_view get_resource_path() const override;
	virtual void destroy_impl() override;
private:
	std::filesystem::path m_path;
	GSharedPtr<IGMeshLoader> m_meshLoader;
	GMeshOut m_loadedMeshes;
	IGResourceManager* m_resManager;
	bool m_tryToRemapMesh;
};

#endif // GMESH_RESOURCE_H