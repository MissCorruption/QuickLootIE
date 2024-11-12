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

		static void BlockConflictingInputs();
		static void UnblockConflictingInputs();

		static void HandleButtonEvent(const RE::ButtonEvent* event);

	private:
		static inline std::vector<Keybinding> _keybindings{};
		static inline RE::stl::enumeration<ModifierKeys> _usedModifiers = ModifierKeys::kNone;
		static inline RE::stl::enumeration<ModifierKeys> _currentModifiers = ModifierKeys::kNone;
		static inline const RE::Setting* _grabDelaySetting = nullptr;
		static inline bool _isHoldingActivate = false;

		static void ReloadKeybindings();
		static Keybinding* FindConflictingKeybinding(const UserEventMapping&, DeviceType);
		static Keybinding* FindMatchingKeybinding(const RE::ButtonEvent* event);

		static void UpdateModifierKeys(const RE::ButtonEvent* event);
		static bool HandleGrab(const RE::ButtonEvent* event, const Keybinding* keybinding);
		static void TryGrab();
		static void TriggerKeybinding(const Keybinding* keybinding);
		static void HandleRetrigger(const RE::ButtonEvent* event, Keybinding* keybinding);

		static void WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts = false);
	};
}
