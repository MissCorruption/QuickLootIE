#pragma once

#include "QuickLootAPI.h"

namespace QuickLoot::Input
{
	// How long a button needs to be held down for a kOnHold keybinding to trigger.
	constexpr auto holdTimeThreshold = 0.4f;
	constexpr auto initialRetriggerDelay = 0.5f;
	constexpr auto subsequentRetriggerDelay = 0.05f;

	using DeviceType = RE::INPUT_DEVICE;
	using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
	using UserEventMapping = RE::ControlMap::UserEventMapping;

	struct VRInputEnum
	{
		enum Keys
		{
			kBY = RE::BSOpenVRControllerDevice::Key::kBY,
			kGrip = RE::BSOpenVRControllerDevice::Key::kGrip,
			kGripAlt = RE::BSOpenVRControllerDevice::Key::kGripAlt,
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

	enum class ButtonArtIndex : uint16_t;

	using QuickLootAction = API::QuickLootAction;

	enum class KeybindingFlags
	{
		kNone = 0,
		// Whether the keybinding should be active while the loot menu is closed.
		kGlobal = 1 << 0,
		// The action should be periodically re-triggered when the button is held.
		kRetrigger = 1 << 1,
		// The action should be triggered when the button is released.
		kOnRelease = 1 << 2,
		// The action should be triggered when the button is held for a minimum amount of time.
		kOnHold = 1 << 3,
	};

	struct Keybinding
	{
		// If a keybinding is part of a group with the kOptional bit set and conflicts with a
		// predefined keybinding, then all keybindings within the same group will be disabled.
		RE::stl::enumeration<ControlGroup> group = ControlGroup::kNone;
		DeviceKey inputKey;
		std::optional<DeviceKey> modifierKey;
		QuickLootAction action;
		RE::stl::enumeration<KeybindingFlags> flags;
		ButtonArtIndex buttonArtOverride = static_cast<ButtonArtIndex>(0);

		// Dynamic information

		float nextRetriggerTime = 0.0f;
		bool isModifierSatisfied = false;
	};

	inline bool IsVrPrimary(DeviceType deviceType)
	{
		return deviceType == DeviceType::kOculusPrimary ||
		       deviceType == DeviceType::kVivePrimary ||
		       deviceType == DeviceType::kWMRPrimary;
	}

	inline bool IsVrSecondary(DeviceType deviceType)
	{
		return deviceType == DeviceType::kOculusSecondary ||
		       deviceType == DeviceType::kViveSecondary ||
		       deviceType == DeviceType::kWMRSecondary;
	}

	inline DeviceKey NormalizeDeviceKey(DeviceType deviceType, uint32_t keyCode)
	{
		bool isVr = false;

		if (IsVrPrimary(deviceType)) {
			deviceType = RE::INPUT_DEVICE::kOculusPrimary;
			isVr = true;
		}

		if (IsVrSecondary(deviceType)) {
			deviceType = RE::INPUT_DEVICE::kOculusSecondary;
			isVr = true;
		}

		if (isVr && keyCode == VRInput::kGripAlt) {
			keyCode = VRInput::kGrip;
		}

		if (deviceType == RE::INPUT_DEVICE::kGamepad &&
			RE::ControlMap::GetSingleton()->GetGamePadType() == RE::PC_GAMEPAD_TYPE::kOrbis) {
			keyCode = SKSE::InputMap::ScePadOffsetToXInput(keyCode);
		}

		return DeviceKey::Get(deviceType, keyCode);
	}
}
