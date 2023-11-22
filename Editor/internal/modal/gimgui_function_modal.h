#ifndef GIMGUI_FUNCTION_MODAL_H
#define GIMGUI_FUNCTION_MODAL_H

#include "editor/igimgui_modal_impl.h"
#include <functional>
#include <string>
class GImGuiFunctionModal : public IGImGuiModalImpl
{
public:
	typedef std::function<bool()> modal_function;
	GImGuiFunctionModal(const std::string& name,modal_function fnc);


	// Inherited via IGImGuiModalImpl
	virtual const char* get_modal_id() const noexcept override;
	virtual void render_modal() override;
	virtual bool wants_to_render() const noexcept override;
private:
	bool m_wantsOpen;
	modal_function m_fnc;
	std::string m_name;

};

#endif // GIMGUI_FUNCTION_MODAL_H