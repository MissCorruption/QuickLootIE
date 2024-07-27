#include "CombatStateObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void CombatStateObserver::Install()
	{
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(GetSingleton());
		logger::info("Installed");
	}

	bool IsPlayerEvent(const RE::TESCombatEvent* a_event)
	{
		if (a_event->actor && a_event->actor->IsPlayerRef())
			return true;

		if (a_event->targetActor && a_event->targetActor->IsPlayerRef())
			return true;

		return false;
	}

	RE::BSEventNotifyControl CombatStateObserver::ProcessEvent(
		const RE::TESCombatEvent* a_event,
		RE::BSTEventSource<RE::TESCombatEvent>*)
	{
		if (a_event && IsPlayerEvent(a_event)) {
			MenuVisibilityManager::OnCombatStateChanged(*a_event->newState);
		}

		return kContinue;
	}
}
