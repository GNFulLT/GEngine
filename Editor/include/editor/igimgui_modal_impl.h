#ifndef IGIMGUI_MODAL_IMPL_H
#define IGIMGUI_MODAL_IMPL_H

class IGImGuiModalImpl
{
public:
	virtual ~IGImGuiModalImpl() = default;

	virtual const char* get_modal_id() const noexcept = 0;

	virtual void render_modal() = 0;

	virtual bool wants_to_render() const noexcept = 0;
private:
};

#endif // IGIMGUI_MODAL_IMPL_H