#pragma once

namespace QuickLoot::Observers
{
	class CameraStateObserver : public RE::BSTEventSink<SKSE::CameraEvent>
	{
		static CameraStateObserver* GetSingleton()
		{
			static CameraStateObserver instance;
			return &instance;
		}

		CameraStateObserver() = default;
		~CameraStateObserver() override = default;
		

	public:
		CameraStateObserver(CameraStateObserver&&) = delete;
		CameraStateObserver(const CameraStateObserver&) = delete;
		CameraStateObserver& operator=(CameraStateObserver&&) = delete;
		CameraStateObserver& operator=(const CameraStateObserver&) = delete;

		static void Install();

        RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* event, RE::BSTEventSource<SKSE::CameraEvent>* a_eventSource) override;
    };
}
