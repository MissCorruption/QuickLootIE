#include "GrabStateObserver.h"

#include "MenuVisibilityManager.h"
#include "RE/B/BSTEvent.h"
#include "RE/S/ScriptEventSourceHolder.h"
#include "RE/T/TESGrabReleaseEvent.h"
#include "SKSE/Impl/PCH.h"

namespace QuickLoot::Observers
{
	void GrabStateObserver::Install()
	{
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(GetSingleton());
		logger::info("Installed {}", typeid(GrabStateObserver).name());
	}

	RE::BSEventNotifyControl GrabStateObserver::ProcessEvent(const RE::TESGrabReleaseEvent*, RE::BSTEventSource<RE::TESGrabReleaseEvent>*)
	{
		MenuVisibilityManager::OnGrabStateChanged();
		return RE::BSEventNotifyControl::kContinue;
	}
}
