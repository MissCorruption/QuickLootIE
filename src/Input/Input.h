#pragma once

namespace QuickLoot::Input
{
	using DeviceType = RE::INPUT_DEVICE;
	using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
	using UserEventMapping = RE::ControlMap::UserEventMapping;

	using KeyboardKey = RE::BSWin32KeyboardDevice::Key;
	using MouseButton = RE::BSWin32MouseDevice::Key;
	using GamepadInput = RE::BSWin32GamepadDevice::Key;

	constexpr UEFlag QUICKLOOT_EVENT_GROUP_FLAG = static_cast<UEFlag>(1 << 12);

	enum class ModifierKeys : uint8_t
	{
		kNone = 0,

		kShift = 1 << 0,
		kControl = 1 << 1,
		kAlt = 1 << 2,

		kIgnore = static_cast<ModifierKeys>(-1),
	};

	enum class ControlGroup : uint8_t
	{
		kNone,
		kOptional = 1 << 7,

		// always active
		kButtonBar = 1,
		kMouseWheel = 2,
		kDpad = 3,

		// disabled on conflict
		kArrowKeys = 11 | kOptional,
		kNumPadArrowKeys = 12 | kOptional,
		kPageKeys = 13 | kOptional,
	};

	enum class QuickLootAction : uint8_t
	{
		kTake,
		kTakeAll,
		kTransfer,
		kScrollUp,
		kScrollDown,
		kPrevPage,
		kNextPage,
	};

	struct Keybinding
	{
		// If a keybinding is part of a group with the kOptional bit set and conflicts with a
		// predefined keybinding, then all keybindings within the same group will be disabled.
		RE::stl::enumeration<ControlGroup> group = ControlGroup::kNone;
		DeviceType deviceType;
		uint16_t inputKey;
		RE::stl::enumeration<ModifierKeys> modifiers;
		QuickLootAction action;
		// Whether the action should be periodically re-triggered when the button is held.
		bool retrigger;
		float nextRetriggerTime = 0.0f;
	};
}
