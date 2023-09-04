#ifndef IKEYBOARD_MANAGER_H
#define IKEYBOARD_MANAGER_H

#include "public/platform/virtual_key_codes.h"

class IKeyboardManager
{
public:
	virtual ~IKeyboardManager() = default;

	virtual bool is_key_pressed(KEY_CODE code) = 0;
	//!: If In one loop key released returns true for once
	virtual bool is_key_release(KEY_CODE code) = 0;
};

#endif // IKEYBOARD_MANAGER_H