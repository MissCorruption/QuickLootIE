#include "CameraStateObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void CameraStateObserver::Install()
	{
		SKSE::GetCameraEventSource()->AddEventSink(GetSingleton());
		logger::info("Installed {}", typeid(CameraStateObserver).name());
	}

	RE::BSEventNotifyControl CameraStateObserver::ProcessEvent(const SKSE::CameraEvent* event, RE::BSTEventSource<SKSE::CameraEvent>*)
	{
		if (event && event->newState) {
			MenuVisibilityManager::OnCameraStateChanged(event->newState->id);
		}

		return kContinue;
	}
}
