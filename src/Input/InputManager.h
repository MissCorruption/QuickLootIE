#pragma once

#include "Input.h"

namespace QuickLoot::Input
{
	class InputManager
	{
	public:
		InputManager() = delete;
		~InputManager() = delete;
		InputManager(InputManager&&) = delete;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		static void Install();
		static void UpdateMappings();

		static void UpdateModifierStates();

		static void BlockConflictingInputs();
		static void UnblockConflictingInputs();

		static void HandleButtonEvent(const RE::ButtonEvent* event);
		static void HandleThumbstickEvent(const RE::ThumbstickEvent* event);

		static std::vector<Keybinding> GetButtonBarKeybindings();

	private:
		static inline std::vector<Keybinding> _keybindings{};
		static inline std::set<DeviceKey> _allInputKeys{};
		static inline std::set<DeviceKey> _allModifierKeys{};
		static inline const RE::Setting* _grabDelaySetting = nullptr;
		static inline bool _triggerOnActivateRelease = false;

		static void ReloadKeybindings();
		static Keybinding* FindConflictingKeybinding(const UserEventMapping&, DeviceType);
		static Keybinding* FindSatisfiedKeybinding(const RE::ButtonEvent* event);

		static bool IsKeyPressed(DeviceKey key);
		static RE::BSInputDevice* GetInputDevice(DeviceType deviceType);

		static void SendFakeButtonEvent(DeviceType device, int idCode, float value, float heldDownSecs);

		static bool HandleGrab(const RE::ButtonEvent* event, const Keybinding* keybinding);
		static bool TryGrab();
		static void TriggerKeybinding(const Keybinding* keybinding);
		static void HandleRetrigger(const RE::ButtonEvent* event, Keybinding* keybinding);

		static void WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts = false);
	};
}
