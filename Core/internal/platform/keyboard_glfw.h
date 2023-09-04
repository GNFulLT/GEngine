#ifndef KEYBOARD_GLFW_H
#define KEYBOARD_GLFW_H

#include "public/platform/ikeyboard_manager.h"

class GLFWKeyboardManager : public IKeyboardManager
{
public:
	GLFWKeyboardManager();
	void key_pressed(KEY_CODE code);

	void key_released(KEY_CODE code);

	// Inherited via IKeyboardManager
	virtual bool is_key_pressed(KEY_CODE code) override;
	virtual bool is_key_release(KEY_CODE code) override;
private:
	bool keys[SUPPORTED_KEY_NUM];
	bool releasedKeys[SUPPORTED_KEY_NUM];
	bool releasedKeyReturned[SUPPORTED_KEY_NUM];

};

#endif // KEYBOARD_GLFW_H