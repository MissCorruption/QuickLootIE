#pragma once

namespace QuickLoot::Observers
{
	class GrabStateObserver : RE::BSTEventSink<RE::TESGrabReleaseEvent>
	{
		static GrabStateObserver* GetSingleton()
		{
			static GrabStateObserver instance;
			return &instance;
		}

		GrabStateObserver() = default;
		~GrabStateObserver() override = default;

	public:
		GrabStateObserver(GrabStateObserver&&) = delete;
		GrabStateObserver(const GrabStateObserver&) = delete;
		GrabStateObserver& operator=(GrabStateObserver&&) = delete;
		GrabStateObserver& operator=(const GrabStateObserver&) = delete;

		static void Install();

		RE::BSEventNotifyControl ProcessEvent(const RE::TESGrabReleaseEvent* event, RE::BSTEventSource<RE::TESGrabReleaseEvent>* eventSource) override;
    };
}
