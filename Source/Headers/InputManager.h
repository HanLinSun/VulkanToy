#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <windows.h>
class InputManager
{
public:
	~InputManager() {};
	InputManager() {};
	InputManager(InputManager&) = delete;
	InputManager& operator = (const InputManager&) = delete;
	std::shared_ptr<InputManager> getInstance()
	{
		if (m_instance_ptr == nullptr)
		{
			std::lock_guard<std::mutex>lock(m_mutex);
			if (m_instance_ptr == nullptr)
			{
				m_instance_ptr = std::shared_ptr<InputManager>(new InputManager);
			}
		}
		return  m_instance_ptr;
	}

	void HandlerPressEvent();

private:

	static std::shared_ptr<InputManager> m_instance_ptr;
	static std::mutex m_mutex;
};