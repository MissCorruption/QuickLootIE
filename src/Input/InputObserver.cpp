#include "InputObserver.h"

#include "InputManager.h"

namespace QuickLoot::Input
{
	void InputObserver::Install()
	{
		RE::BSInputDeviceManager::GetSingleton()->AddEventSink(GetSingleton());

		// We might have missed modifier key changes while input wasn't listening.
		InputManager::UpdateModifierStates();

		logger::info("Installed {}", typeid(InputObserver).name());
	}

	RE::BSEventNotifyControl InputObserver::ProcessEvent(RE::InputEvent* const* event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		if (*event) {
			for (auto current = *event; current; current = current->next) {
				if (auto buttonEvent = current->AsButtonEvent()) {
					InputManager::HandleButtonEvent(buttonEvent);
				}
				if (auto thumbStickEvent = current->AsThumbstickEvent()) {
					InputManager::HandleThumbstickEvent(thumbStickEvent);
				}
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
