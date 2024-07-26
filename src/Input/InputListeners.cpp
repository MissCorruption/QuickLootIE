#include "Input/InputListeners.h"

#include "Input.h"
#include "LootMenuManager.h"

namespace Input
{
	ScrollHandler::ScrollHandler()
	{
		using Device = RE::INPUT_DEVICE;
		using Gamepad = RE::BSWin32GamepadDevice::Key;
		using Keyboard = RE::BSWin32KeyboardDevice::Key;
		using Mouse = RE::BSWin32MouseDevice::Key;

		const auto& groups = ControlGroups::get();

		if (groups[Group::kPageKeys]) {
			auto& mappings = _mappings[Device::kKeyboard];
			mappings.emplace(Keyboard::kPageUp, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Keyboard::kPageDown, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(1.0); });
		}

		if (groups[Group::kArrowKeys]) {
			auto& mappings = _mappings[Device::kKeyboard];
			mappings.emplace(Keyboard::kUp, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Keyboard::kDown, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(1.0); });
			mappings.emplace(Keyboard::kLeft, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Keyboard::kRight, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(1.0); });
		}

		if (groups[Group::kMouseWheel]) {
			auto& mappings = _mappings[Device::kMouse];
			mappings.emplace(Mouse::kWheelUp, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Mouse::kWheelDown, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(1.0); });
		}

		if (groups[Group::kDPAD]) {
			auto& mappings = _mappings[Device::kGamepad];
			mappings.emplace(Gamepad::kUp, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(-1.0); });
			mappings.emplace(Gamepad::kDown, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedIndex(1.0); });
			mappings.emplace(Gamepad::kLeft, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(-1.0); });
			mappings.emplace(Gamepad::kRight, [] { QuickLoot::LootMenuManager::GetSingleton().ModSelectedPage(1.0); });
		}
	}

	void TakeHandler::TakeStack()
	{
		auto& loot = QuickLoot::LootMenuManager::GetSingleton();
		loot.TakeStack();
	}

	void TakeHandler::TryGrab()
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		player->StartGrabObject();
		if (!player->IsGrabbing()) {
			return;
		}

		auto playerControls = RE::PlayerControls::GetSingleton();
		auto activateHandler = playerControls ? playerControls->GetActivateHandler() : nullptr;
		if (activateHandler) {
			activateHandler->SetHeldButtonActionSuccess(true);
		}

		auto& loot = QuickLoot::LootMenuManager::GetSingleton();
		loot.Close();
	}

	// TODO: Actually take all items
	void TakeHandler::TakeAll()
	{
		auto& loot = QuickLoot::LootMenuManager::GetSingleton();
		loot.TakeAll();
	}

	void TransferHandler::DoHandle(RE::InputEvent* const& a_event)
	{
		for (auto iter = a_event; iter; iter = iter->next) {
			auto event = iter->AsButtonEvent();
			if (!event) {
				continue;
			}

			auto controlMap = RE::ControlMap::GetSingleton();
			const auto idCode =
				controlMap ?
                    controlMap->GetMappedKey("Favorites"sv, event->GetDevice()) :
                    RE::ControlMap::kInvalid;

			if (event->GetIDCode() == idCode && event->IsDown()) {
				auto player = RE::PlayerCharacter::GetSingleton();
				if (player) {
					player->ActivatePickRef();
				}

				auto& loot = QuickLoot::LootMenuManager::GetSingleton();
				loot.Close();
				return;
			}
		}
	}
}
