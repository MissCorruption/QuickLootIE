#include "CrosshairRefObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void CrosshairRefObserver::Install()
	{
		SKSE::GetCrosshairRefEventSource()->AddEventSink(GetSingleton());
		logger::info("Installed");
	}

	RE::BSEventNotifyControl CrosshairRefObserver::ProcessEvent(
		const SKSE::CrosshairRefEvent* a_event,
		RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
		auto handle = RE::ObjectRefHandle();

		if (a_event && a_event->crosshairRef) {
			handle = a_event->crosshairRef->CreateRefHandle();
		}

		MenuVisibilityManager::OnCrosshairRefChanged(handle);

		return kContinue;
	}
}
