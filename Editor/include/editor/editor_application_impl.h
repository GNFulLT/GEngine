#ifndef EDITOR_APPLICATION_IMPL
#define EDITOR_APPLICATION_IMPL

#include "engine/gapplication_impl.h"
#include "editor/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"

class GEngine;
class ImGuiLayer;
class IOwningGLogger;

class EditorApplicationImpl : public GApplicationImpl
{
public:
	static EditorApplicationImpl* get_instance();
	
	virtual void destroy() override;
	
	virtual bool before_update() override;

	virtual void update() override;

	virtual void after_update() override;

	virtual bool before_render() override;

	virtual void render() override;

	virtual void after_render() override;

	virtual bool init(GEngine* engine) override;

	GEngine* m_engine;

	GSharedPtr<IOwningGLogger> get_editor_logger();
private:
	ImGuiLayer* m_imguiLayer;
	GSharedPtr<IOwningGLogger> m_logger;
	inline static EditorApplicationImpl* s_instance;
};


extern EDITOR_API GApplicationImpl* create_the_editor();
#endif // EDITOR_APPLICATION_IMPL