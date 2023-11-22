#ifndef GIMGUI_MODAL_H
#define GIMGUI_MODAL_H

#include "editor/igimgui_modal_impl.h"

class GImGuiModal
{
public:
	GImGuiModal(IGImGuiModalImpl* impl);
	~GImGuiModal();
	bool render_modal();
private:
	IGImGuiModalImpl* m_impl;
	bool m_isModalOpen;
};

#endif // GIMGUI_MODAL_H