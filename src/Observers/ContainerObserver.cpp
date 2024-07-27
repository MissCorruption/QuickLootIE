#include "ContainerObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void ContainerObserver::Install()
	{
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(GetSingleton());
		logger::info("Installed");
	}

	RE::BSEventNotifyControl ContainerObserver::ProcessEvent(const RE::TESContainerChangedEvent* event, RE::BSTEventSource<RE::TESContainerChangedEvent>*)
	{
		if (!event) {
			return kContinue;
		}

		if (event->oldContainer) {
			MenuVisibilityManager::OnContainerChanged(event->oldContainer);
		}

		if (event->newContainer) {
			MenuVisibilityManager::OnContainerChanged(event->newContainer);
		}

		return kContinue;
	}
}
