#pragma once

#include <atomic>

namespace QuickLoot::Observers
{
	class CrosshairRefObserver : public RE::BSTEventSink<SKSE::CrosshairRefEvent>
	{
		static CrosshairRefObserver* GetSingleton()
		{
			static CrosshairRefObserver instance;
			return &instance;
		}

		CrosshairRefObserver() = default;
		~CrosshairRefObserver() override = default;

	public:
		CrosshairRefObserver(CrosshairRefObserver&&) = delete;
		CrosshairRefObserver(const CrosshairRefObserver&) = delete;
		CrosshairRefObserver& operator=(CrosshairRefObserver&&) = delete;
		CrosshairRefObserver& operator=(const CrosshairRefObserver&) = delete;

		static void StartVRTargetPollThread();
		static void StopVRTargetPollThread();
		static void Install();

		RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event, RE::BSTEventSource<SKSE::CrosshairRefEvent>* a_eventSource) override;

	private:
		static inline std::atomic<bool> _vrPollRunning{ false };
	};
}
