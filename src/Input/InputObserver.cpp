#include "InputObserver.h"

#include "InputManager.h"

namespace QuickLoot::Input
{
	void InputObserver::StartListening()
	{
		if (!_isListening) {
			_isListening = true;
			RE::BSInputDeviceManager::GetSingleton()->AddEventSink(GetSingleton());
			logger::debug("{}", __func__);

			// We might have missed modifier key changes while input wasn't listening.
			InputManager::UpdateModifierKeys();
		}
	}

	void InputObserver::StopListening()
	{
		if (_isListening) {
			_isListening = false;
			RE::BSInputDeviceManager::GetSingleton()->RemoveEventSink(GetSingleton());
			logger::debug("{}", __func__);
		}
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
