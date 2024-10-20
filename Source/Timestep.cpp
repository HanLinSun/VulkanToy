#include <Timestep.h>

namespace Renderer
{

	static std::shared_ptr<Timestep> m_instancePtr = nullptr;

	Timestep::Timestep() {}

	Timestep::~Timestep()
	{
		m_instancePtr.reset();
	}

	float Timestep::GetSeconds() const { return m_deltaTime; }
	float Timestep::GetMilliseconds() const { return m_deltaTime * 1000.f; }

	void Timestep::SetDeltaTime(float deltaTime)
	{
		m_deltaTime = deltaTime;
	}

	std::shared_ptr<Timestep> Timestep::GetInstance()
	{
		if (m_instancePtr == nullptr) {
			m_instancePtr = std::make_shared<Timestep>();
		}
		return m_instancePtr;
	}
}
