#include "ContainerChangedHandler.h"

#include "LootMenuManager.h"

auto ContainerChangedHandler::ProcessEvent(const RE::TESContainerChangedEvent* a_event, RE::BSTEventSource<RE::TESContainerChangedEvent>*)
	-> EventResult
{
	auto container = _container.get();
	if (a_event &&
		container &&
		(a_event->oldContainer == container->GetFormID() ||
			a_event->newContainer == container->GetFormID())) {
		QuickLoot::LootMenuManager::RefreshInventory();
	}

	return EventResult::kContinue;
}
