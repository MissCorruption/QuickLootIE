#include "LockChangedObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void LockChangedObserver::Install()
	{
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(GetSingleton());
		logger::info("Installed");
	}

	RE::BSEventNotifyControl LockChangedObserver::ProcessEvent(
		const RE::TESLockChangedEvent* a_event,
		RE::BSTEventSource<RE::TESLockChangedEvent>*)
	{
		if (a_event && a_event->lockedObject) {
			MenuVisibilityManager::OnLockChanged(*a_event->lockedObject);
		}

		return kContinue;
	}
}
