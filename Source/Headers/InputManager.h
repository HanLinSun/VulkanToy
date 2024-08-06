#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <windows.h>
#include <SceneStructs.h>

class InputManager
{
public:
	~InputManager() {};
	InputManager() {};
	InputManager(InputManager&) = delete;
	InputManager& operator = (const InputManager&) = delete;

	void HandlePressEvent(Renderer::CamInputListener& inputListener);
	void KeyEventHelper(char keycode, bool& val);
	void MouseClickEventHelper();
	void MouseScrollEventHelper();
private:

	//static std::shared_ptr<InputManager> m_instance_ptr;
	static std::mutex m_mutex;
};