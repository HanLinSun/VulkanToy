#pragma once
#include <vector>
#include <thread>
#include <Queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace Renderer
{
	class Thread
	{
	private:
		bool m_destroying = false;
		std::thread m_worker;
		std::queue<std::function<void()>> m_jobQueue;
		std::mutex m_queueMutex;
		std::condition_variable m_condition;

		void QueueLoop()
		{
			while (true)
			{
				std::function<void()> job;
				{
					std::unique_lock<std::mutex> lock(m_queueMutex);
					m_condition.wait(lock, [this] {return !m_jobQueue.empty() || m_destroying; });
					if (m_destroying)
					{
						break;
					}
					job = m_jobQueue.front();
				}

				job();

				//job is done, release the job
				{
					std::lock_guard<std::mutex> lock(m_queueMutex);
					m_jobQueue.pop();
					m_condition.notify_one();
				}
			}
		}

	public:
		Thread()
		{
			m_worker = std::thread(&Thread::QueueLoop, this);
		}

		~Thread()
		{
			if (m_worker.joinable())
			{
				Wait();
				m_queueMutex.lock();
				m_destroying = true;
				m_condition.notify_one();
				m_queueMutex.unlock();
				m_worker.join();
			}
		}

		void AddJob(std::function<void()> function)
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_jobQueue.push(std::move(function));
			m_condition.notify_one();
		}

		// Wait until all work items have been finished
		void Wait()
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_condition.wait(lock, [this]() { return m_jobQueue.empty(); });
		}
	};

	class ThreadPool
	{
	public:
		std::vector<std::unique_ptr<Thread>> m_threads;

		void SetThreadCount(uint32_t count)
		{
			m_threads.clear();
			for (uint32_t i = 0; i < count; i++)
			{
				m_threads.push_back(std::make_unique<Thread>());
			}
		}

		void Wait()
		{
			for (auto& thread : m_threads)
			{
				thread->Wait();
			}
		}
	};
}