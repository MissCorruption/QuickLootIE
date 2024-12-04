#include "Settings.h"

#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"
#include "Input/InputManager.h"

namespace QuickLoot::Config
{
	void Settings::Update()
	{
		Papyrus::UpdateVariables();

		if (ShowWhenUnlocked()) {
			Behaviors::LockpickActivation::Block();
		} else {
			Behaviors::LockpickActivation::Unblock();
		}

		Input::InputManager::UpdateMappings();
	}

	bool Settings::ShowInCombat() { return QLIE_ShowInCombat; }
	bool Settings::ShowWhenEmpty() { return QLIE_ShowWhenEmpty; }
	bool Settings::ShowWhenUnlocked() { return QLIE_ShowWhenUnlocked; }
	bool Settings::ShowInThirdPersonView() { return QLIE_ShowInThirdPerson; }
	bool Settings::ShowWhenMounted() { return QLIE_ShowWhenMounted; }
	bool Settings::EnableForAnimals() { return QLIE_EnableForAnimals; }
	bool Settings::EnableForDragons() { return QLIE_EnableForDragons; }
	bool Settings::DispelInvisibility() { return QLIE_BreakInvisibility; }

	int Settings::GetWindowX() { return QLIE_WindowOffsetX; }
	int Settings::GetWindowY() { return QLIE_WindowOffsetY; }
	float Settings::GetWindowScale() { return QLIE_WindowScale; }
	AnchorPoint Settings::GetWindowAnchor() { return static_cast<AnchorPoint>(QLIE_WindowAnchor); }
	int Settings::GetWindowMinLines() { return QLIE_WindowMinLines; }
	int Settings::GetWindowMaxLines() { return QLIE_WindowMaxLines; }
	float Settings::GetWindowOpacityNormal() { return QLIE_WindowOpacityNormal; }
	float Settings::GetWindowOpacityEmpty() { return QLIE_WindowOpacityEmpty; }

	bool Settings::ShowIconRead() { return QLIE_ShowIconRead; }
	bool Settings::ShowIconStolen() { return QLIE_ShowIconStolen; }
	bool Settings::ShowIconEnchanted() { return QLIE_ShowIconEnchanted; }
	bool Settings::ShowIconEnchantedKnown() { return QLIE_ShowIconEnchantedKnown; }
	bool Settings::ShowIconEnchantedSpecial() { return QLIE_ShowIconEnchantedSpecial; }

	std::vector<std::string> Settings::GetInfoColumns() { return QLIE_InfoColumns; }

	const std::vector<std::string>& Settings::GetUserDefinedSortPriority() { return QLIE_SortRulesActive; }

	std::vector<Input::Keybinding> Settings::GetKeybindings()
	{
		std::vector keybindings{
			GetKeybinding(QLIE_KeybindingTake, QLIE_KeybindingTakeModifier, Input::QuickLootAction::kTake),
			GetKeybinding(QLIE_KeybindingTakeAll, QLIE_KeybindingTakeAllModifier, Input::QuickLootAction::kTakeAll),
			GetKeybinding(QLIE_KeybindingTransfer, QLIE_KeybindingTransferModifier, Input::QuickLootAction::kTransfer),
			GetKeybinding(QLIE_KeybindingTakeGamepad, 0, Input::QuickLootAction::kTake),
			GetKeybinding(QLIE_KeybindingTakeAllGamepad, 0, Input::QuickLootAction::kTakeAll),
			GetKeybinding(QLIE_KeybindingTransferGamepad, 0, Input::QuickLootAction::kTransfer),
		};

		// Remove unmapped keybindings.
		std::erase_if(keybindings, [](const auto& keybinding) { return keybinding.inputKey == static_cast<uint16_t>(-1); });

		return keybindings;
	}

	bool Settings::ShowArtifactDisplayed() { return QLIE_ShowIconArtifactDisplayed; }
	bool Settings::ShowArtifactFound() { return QLIE_ShowIconArtifactCarried; }
	bool Settings::ShowArtifactNew() { return QLIE_ShowIconArtifactNew; }
	bool Settings::ShowCompletionistNeeded() { return QLIE_ShowIconCompletionistNeeded; }
	bool Settings::ShowCompletionistCollected() { return QLIE_ShowIconCompletionistCollected; }

	Input::Keybinding Settings::GetKeybinding(int skseKey, int modifierType, Input::QuickLootAction action)
	{
		Input::ModifierKeys modifiers = ModifierTypeToModifierKeys(modifierType);
		Input::DeviceType deviceType;
		uint16_t keyCode;
		SkseKeyToDeviceKey(skseKey, deviceType, keyCode);

		return Input::Keybinding{ Input::ControlGroup::kButtonBar, deviceType, keyCode, modifiers, action, false };
	}

	Input::ModifierKeys Settings::ModifierTypeToModifierKeys(int modifierType)
	{
		switch (modifierType) {
		case 1:
			return Input::ModifierKeys::kNone;
		case 2:
			return Input::ModifierKeys::kShift;
		case 3:
			return Input::ModifierKeys::kControl;
		case 4:
			return Input::ModifierKeys::kAlt;
		default:
			return Input::ModifierKeys::kIgnore;
		}
	}

	void Settings::SkseKeyToDeviceKey(int skseKey, Input::DeviceType& deviceType, uint16_t& keyCode)
	{
		if (skseKey >= 266) {
			deviceType = Input::DeviceType::kGamepad;
			keyCode = static_cast<uint16_t>(SKSE::InputMap::GamepadKeycodeToMask(skseKey));
		} else if (skseKey >= 256) {
			deviceType = Input::DeviceType::kMouse;
			keyCode = static_cast<uint16_t>(skseKey - 256);
		} else {
			deviceType = Input::DeviceType::kKeyboard;
			keyCode = static_cast<uint16_t>(skseKey);
		}
	}
}
