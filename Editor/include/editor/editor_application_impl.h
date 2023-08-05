#ifndef EDITOR_APPLICATION_IMPL
#define EDITOR_APPLICATION_IMPL

#include "engine/gapplication_impl.h"
#include "editor/GEngine_EXPORT.h"

class GEngine;
class ImGuiLayer;

class EditorApplicationImpl : public GApplicationImpl
{
public:
	virtual void destroy() override;


	virtual bool before_update() override;

	virtual void update() override;

	virtual void after_update() override;

	virtual bool before_render() override;

	virtual void render() override;

	virtual void after_render() override;

	virtual bool init(GEngine* engine) override;
private:
	GEngine* m_engine;
	ImGuiLayer* m_imguiLayer;
};


extern EDITOR_API GApplicationImpl* create_the_editor();
#endif // EDITOR_APPLICATION_IMPL