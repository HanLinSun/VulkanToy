#pragma once

#include <memory>
namespace Renderer
{
	class Timestep
	{
	public:
		Timestep();
		~Timestep();

		float GetSeconds() const;
		float GetMilliseconds() const;

		void SetDeltaTime(float deltaTime);

		static std::shared_ptr<Timestep> GetInstance();

	private:
		float m_deltaTime=0.0f;
	};
}