#pragma once
#include "Input/Input.h"

namespace QuickLoot::Config
{
	enum AnchorPoint : uint8_t
	{
		kTopLeft,
		kCenterLeft,
		kBottomLeft,
		kTopCenter,
		kCenter,
		kBottomCenter,
		kTopRight,
		kCenterRight,
		kBottomRight,
	};

	class Settings
	{
	public:
		Settings() = delete;
		~Settings() = delete;
		Settings(Settings const&) = delete;
		Settings(Settings const&&) = delete;
		Settings operator=(Settings&) = delete;
		Settings operator=(Settings&&) = delete;

		static void Update();

		static bool ShowInCombat();
		static bool ShowWhenEmpty();
		static bool ShowWhenUnlocked();
		static bool ShowInThirdPersonView();
		static bool ShowWhenMounted();
		static bool EnableForAnimals();
		static bool EnableForDragons();
		static bool DispelInvisibility();

		static int GetWindowX();
		static int GetWindowY();
		static float GetWindowScale();
		static AnchorPoint GetWindowAnchor();
		static int GetWindowMinLines();
		static int GetWindowMaxLines();
		static float GetWindowOpacityNormal();
		static float GetWindowOpacityEmpty();

		static bool ShowIconRead();
		static bool ShowIconStolen();
		static bool ShowIconEnchanted();
		static bool ShowIconEnchantedKnown();
		static bool ShowIconEnchantedSpecial();

		static std::vector<std::string> GetInfoColumns();

		static const std::vector<std::string>& GetUserDefinedSortPriority();

		static std::vector<Input::Keybinding> GetKeybindings();

		static bool ShowArtifactDisplayed();
		static bool ShowArtifactFound();
		static bool ShowArtifactNew();
		static bool ShowCompletionistNeeded();
		static bool ShowCompletionistCollected();

	private:
		static Input::Keybinding BuildKeybinding(Input::ControlGroup group, Input::QuickLootAction action, int skseKey, int modifierType);
		static Input::ModifierKeys ModifierTypeToModifierKeys(int modifierType);
		static void SkseKeyToDeviceKey(int skseKey, Input::DeviceType& deviceType, uint16_t& keyCode);
	};
}
