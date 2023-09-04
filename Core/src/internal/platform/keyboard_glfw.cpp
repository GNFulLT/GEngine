#include "internal/platform/keyboard_glfw.h"

GLFWKeyboardManager::GLFWKeyboardManager()
{
	for (int i = 0; i < SUPPORTED_KEY_NUM; i++)
	{
		keys[i] = false;
		releasedKeys[i] = false;
		releasedKeyReturned[i] = false;
	}
}

void GLFWKeyboardManager::key_pressed(KEY_CODE code)
{
	keys[code] = true;
}

void GLFWKeyboardManager::key_released(KEY_CODE code)
{
	keys[code] = false;
	releasedKeys[code] = true;
}

bool GLFWKeyboardManager::is_key_pressed(KEY_CODE code)
{
	return keys[code];
}

bool GLFWKeyboardManager::is_key_release(KEY_CODE code)
{
	if (releasedKeys[code])
	{
		if (releasedKeyReturned[code])
		{
			releasedKeys[code] = false;
			releasedKeyReturned[code] = false;
			return false;
		}
		else
		{
			releasedKeyReturned[code] = true;
			return true;
		}
	}
	else
	{
		return false;
	}
}
