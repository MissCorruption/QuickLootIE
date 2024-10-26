#include "MenuObserver.h"

#include "MenuVisibilityManager.h"
#include "RE/M/MenuOpenCloseEvent.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	void MenuObserver::Install()
	{
		RE::UI::GetSingleton()->AddEventSink(GetSingleton());
		logger::info("Installed");
	}

	RE::BSEventNotifyControl MenuObserver::ProcessEvent(
		const RE::MenuOpenCloseEvent* event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (event) {
			MenuVisibilityManager::OnMenuOpenClose(event->opening, event->menuName);
		}

		return kContinue;
	}
}
