#pragma once

namespace QuickLoot::Input
{
	using DeviceType = RE::INPUT_DEVICE;
	using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
	using UserEventMapping = RE::ControlMap::UserEventMapping;

	struct VRInputEnum
	{
		enum Keys
		{
			kBY = RE::BSOpenVRControllerDevice::Key::kBY,
			kGrip = RE::BSOpenVRControllerDevice::Key::kGrip,
			kXA = RE::BSOpenVRControllerDevice::Key::kXA,
			kJoystickTrigger = RE::BSOpenVRControllerDevice::Key::kJoystickTrigger,
			kTrigger = RE::BSOpenVRControllerDevice::Key::kTrigger,

			// fake button ids to handle thumb stick events
			kMainThumbStickUp = 1001,
			kMainThumbStickDown = 1002,
		};
	};

	using KeyboardKey = RE::BSWin32KeyboardDevice::Key;
	using MouseButton = RE::BSWin32MouseDevice::Key;
	using GamepadInput = RE::BSWin32GamepadDevice::Key;
	using VRInput = VRInputEnum::Keys;

	constexpr UEFlag QUICKLOOT_EVENT_GROUP_FLAG = static_cast<UEFlag>(1 << 12);

	struct DeviceKey
	{
		DeviceType deviceType;
		uint32_t keyCode;

		auto operator<=>(const DeviceKey& deviceKey) const = default;

		static constexpr DeviceKey Get(DeviceType deviceType, uint32_t keyCode)
		{
			return { deviceType, keyCode };
		}

		static constexpr DeviceKey Get(KeyboardKey keyCode)
		{
			return Get(RE::INPUT_DEVICE::kKeyboard, keyCode);
		}

		static constexpr DeviceKey Get(MouseButton keyCode)
		{
			return Get(RE::INPUT_DEVICE::kMouse, keyCode);
		}

		static constexpr DeviceKey Get(GamepadInput keyCode)
		{
			return Get(RE::INPUT_DEVICE::kGamepad, keyCode);
		}
	};

	enum class ControlGroup : uint8_t
	{
		kNone,
		kOptional = 1 << 7,

		// always active
		kButtonBar = 1,
		kMouseWheel = 2,
		kDpad = 3,
		kVrScroll = 4,
		kEnableState = 5,

		// disabled on conflict
		kArrowKeys = 11 | kOptional,
		kNumPadArrowKeys = 12 | kOptional,
		kPageKeys = 13 | kOptional,
	};

	enum class QuickLootAction : uint8_t
	{
		kDisable,
		kEnable,

		kUse,
		kTake,
		kTakeAll,
		kTransfer,

		kScrollUp,
		kScrollDown,
		kPrevPage,
		kNextPage,
	};

	enum class ButtonArtIndex : uint16_t;

	struct Keybinding
	{
		// If a keybinding is part of a group with the kOptional bit set and conflicts with a
		// predefined keybinding, then all keybindings within the same group will be disabled.
		RE::stl::enumeration<ControlGroup> group = ControlGroup::kNone;
		DeviceKey inputKey;
		std::optional<DeviceKey> modifierKey;
		QuickLootAction action;
		// Whether the action should be periodically re-triggered when the button is held.
		bool retrigger;
		// Whether the keybinding should be active while the loot menu is closed.
		bool global = false;
		ButtonArtIndex buttonArtOverride = static_cast<ButtonArtIndex>(0);

		// Dynamic information

		float nextRetriggerTime = 0.0f;
		bool isModifierSatisfied = false;
	};
}
