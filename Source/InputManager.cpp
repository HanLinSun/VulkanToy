#include "Headers/InputManager.h"

void InputManager::KeyEventHelper(char keycode, bool& val)
{
	if (GetKeyState(keycode)<0)
	{
		val=false;
	}
	else
	{
		val = true;
	}
}

void InputManager::HandlePressEvent(Renderer::CamInputListener& inputListener)
{

}