#include "UserSettings.h"

#include "Behaviors/LockpickActivation.h"
#include "Config/Papyrus.h"
#include "Input/InputManager.h"

namespace QuickLoot::Config
{
	void UserSettings::Update()
	{
		Papyrus::UpdateVariables();

		if (ShowWhenUnlocked()) {
			Behaviors::LockpickActivation::Block();
		} else {
			Behaviors::LockpickActivation::Unblock();
		}

		Input::InputManager::UpdateMappings();
	}

	bool UserSettings::ShowInCombat() { return QLIE_ShowInCombat; }
	bool UserSettings::ShowWhenEmpty() { return QLIE_ShowWhenEmpty; }
	bool UserSettings::ShowWhenUnlocked() { return QLIE_ShowWhenUnlocked; }
	bool UserSettings::ShowInThirdPersonView() { return QLIE_ShowInThirdPerson; }
	bool UserSettings::ShowWhenMounted() { return QLIE_ShowWhenMounted; }
	bool UserSettings::EnableForCorpses() { return QLIE_EnableForCorpses; }
	bool UserSettings::EnableForAnimals() { return QLIE_EnableForAnimals; }
	bool UserSettings::EnableForDragons() { return QLIE_EnableForDragons; }
	bool UserSettings::DispelInvisibility() { return QLIE_BreakInvisibility; }
	bool UserSettings::PlayScrollSound() { return QLIE_PlayScrollSound; }

	int UserSettings::GetWindowX() { return QLIE_WindowOffsetX; }
	int UserSettings::GetWindowY() { return QLIE_WindowOffsetY; }
	float UserSettings::GetWindowScale() { return QLIE_WindowScale; }
	AnchorPoint UserSettings::GetWindowAnchor() { return static_cast<AnchorPoint>(QLIE_WindowAnchor); }
	int UserSettings::GetWindowMinLines() { return QLIE_WindowMinLines; }
	int UserSettings::GetWindowMaxLines() { return QLIE_WindowMaxLines; }
	float UserSettings::GetWindowOpacityNormal() { return QLIE_WindowOpacityNormal; }
	float UserSettings::GetWindowOpacityEmpty() { return QLIE_WindowOpacityEmpty; }

	bool UserSettings::ShowIconItem() { return QLIE_ShowIconItem; }
	bool UserSettings::ShowIconBest() { return QLIE_ShowIconBest; }
	bool UserSettings::ShowIconRead() { return QLIE_ShowIconRead; }
	bool UserSettings::ShowIconStolen() { return QLIE_ShowIconStolen; }
	bool UserSettings::ShowIconEnchanted() { return QLIE_ShowIconEnchanted; }
	bool UserSettings::ShowIconEnchantedKnown() { return QLIE_ShowIconEnchantedKnown; }
	bool UserSettings::ShowIconEnchantedSpecial() { return QLIE_ShowIconEnchantedSpecial; }

	std::vector<std::string> UserSettings::GetInfoColumns() { return QLIE_InfoColumns; }

	const std::vector<std::string>& UserSettings::GetActiveSortRules() { return QLIE_SortRulesActive; }

	std::vector<Input::Keybinding> UserSettings::GetKeybindings()
	{
		std::vector keybindings{
			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTake, QLIE_KeybindingTake, QLIE_KeybindingTakeModifier),
			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTakeAll, QLIE_KeybindingTakeAll, QLIE_KeybindingTakeAllModifier),
			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTransfer, QLIE_KeybindingTransfer, QLIE_KeybindingTransferModifier),
			BuildKeybinding(Input::ControlGroup::kEnableState, Input::QuickLootAction::kDisable, QLIE_KeybindingDisable, QLIE_KeybindingDisableModifier),
			BuildKeybinding(Input::ControlGroup::kEnableState, Input::QuickLootAction::kEnable, QLIE_KeybindingEnable, QLIE_KeybindingEnableModifier),

			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTake, QLIE_KeybindingTakeGamepad, QLIE_KeybindingTakeGamepadModifier),
			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTakeAll, QLIE_KeybindingTakeAllGamepad, QLIE_KeybindingTakeAllGamepadModifier),
			BuildKeybinding(Input::ControlGroup::kButtonBar, Input::QuickLootAction::kTransfer, QLIE_KeybindingTransferGamepad, QLIE_KeybindingTransferGamepadModifier),
			BuildKeybinding(Input::ControlGroup::kEnableState, Input::QuickLootAction::kDisable, QLIE_KeybindingDisableGamepad, QLIE_KeybindingDisableGamepadModifier),
			BuildKeybinding(Input::ControlGroup::kEnableState, Input::QuickLootAction::kEnable, QLIE_KeybindingEnableGamepad, QLIE_KeybindingEnableGamepadModifier),
		};

		// Remove unmapped keybindings.
		std::erase_if(keybindings, [](const auto& keybinding) { return keybinding.inputKey.keyCode == static_cast<uint32_t>(-1); });

		return keybindings;
	}

	bool UserSettings::ShowArtifactDisplayed() { return QLIE_ShowIconArtifactDisplayed; }
	bool UserSettings::ShowArtifactFound() { return QLIE_ShowIconArtifactCarried; }
	bool UserSettings::ShowArtifactNew() { return QLIE_ShowIconArtifactNew; }

	bool UserSettings::ShowCompletionistNeeded() { return QLIE_ShowIconCompletionistNeeded; }
	bool UserSettings::ShowCompletionistCollected() { return QLIE_ShowIconCompletionistCollected; }
	bool UserSettings::ShowCompletionistDisplayed() { return QLIE_ShowIconCompletionistDisplayed; }
	bool UserSettings::ShowCompletionistOccupied() { return QLIE_ShowIconCompletionistOccupied; }
	bool UserSettings::ShowCompletionistDisplayable() { return QLIE_ShowIconCompletionistDisplayable; }

	// TODO
	float UserSettings::VrOffsetX() { return -10; }
	float UserSettings::VrOffsetY() { return 5; }
	float UserSettings::VrOffsetZ() { return 0; }
	float UserSettings::VrAngleX() { return 45; }
	float UserSettings::VrAngleY() { return 0; }
	float UserSettings::VrAngleZ() { return 0; }
	float UserSettings::VrScale() { return 50; }

	Input::Keybinding UserSettings::BuildKeybinding(Input::ControlGroup group, Input::QuickLootAction action, int skseInputKey, int skseModifierKey)
	{
		Input::DeviceKey inputKey = SkseKeyToDeviceKey(skseInputKey);
		std::optional<Input::DeviceKey> modifierKey{};

		if (skseModifierKey > 0) {
			modifierKey = { SkseKeyToDeviceKey(skseModifierKey) };
		}

		const bool global = group == Input::ControlGroup::kEnableState;

		return Input::Keybinding{ group, inputKey, modifierKey, action, false, global };
	}

	Input::DeviceKey UserSettings::SkseKeyToDeviceKey(int skseKey)
	{
		if (skseKey >= 266) {
			return {
				.deviceType = Input::DeviceType::kGamepad,
				.keyCode = static_cast<uint16_t>(SKSE::InputMap::GamepadKeycodeToMask(skseKey))
			};
		}

		if (skseKey >= 256) {
			return {
				.deviceType = Input::DeviceType::kMouse,
				.keyCode = static_cast<uint16_t>(skseKey - 256)
			};
		}

		return {
			.deviceType = Input::DeviceType::kKeyboard,
			.keyCode = static_cast<uint16_t>(skseKey)
		};
	}
}
