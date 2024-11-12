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

	private:
		static inline std::vector<Keybinding> _keybindings{};

		static void ReloadKeybindings();
		static Keybinding* FindConflictingKeybinding(const UserEventMapping&, DeviceType);
		static void WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts = false);
	};
}
