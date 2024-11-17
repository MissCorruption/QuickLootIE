#include "Input/ButtonArt.h"

namespace QuickLoot::Input
{
	ButtonArtIndex ButtonArt::GetFrameIndexForDeviceKey(DeviceType deviceType, uint16_t keyCode)
	{
		Initialize();

		switch (deviceType) {
		case DeviceType::kKeyboard:
			return GetFrameIndexForKeyboardKey(keyCode);

		case DeviceType::kMouse:
			return GetFrameIndexForMouseButton(keyCode);

		case DeviceType::kGamepad:
			return GetFrameIndexForGamepadInput(keyCode);

		default:
			return ButtonArtIndex::kNoMapping;
		}
	}

	ButtonArtIndex ButtonArt::GetFrameIndexForEvent(const std::string_view& event)
	{
		Initialize();

		const auto input = RE::BSInputDeviceManager::GetSingleton();
		const auto controlMap = RE::ControlMap::GetSingleton();

		if (!input || !controlMap) {
			return ButtonArtIndex::kNoMapping;
		}

		if (input->IsGamepadEnabled()) {
			return GetFrameIndexForGamepadInput(LOWORD(controlMap->GetMappedKey(event, RE::INPUT_DEVICE::kGamepad)));
		}

		const auto index = GetFrameIndexForKeyboardKey(LOWORD(controlMap->GetMappedKey(event, RE::INPUT_DEVICE::kKeyboard)));

		if (index != ButtonArtIndex::kNoMapping) {
			return index;
		}

		return GetFrameIndexForMouseButton(LOWORD(controlMap->GetMappedKey(event, RE::INPUT_DEVICE::kMouse)));
	}

	ButtonArtIndex ButtonArt::GetFrameIndexForKeyboardKey(uint16_t keyCode)
	{
		// Keyboard scan codes correspond 1:1 to frame indices.
		// The bit set contains a 1 if the corresponding key code is valid.

		if (keyCode >= _keySet.size() || !_keySet.test(keyCode)) {
			return ButtonArtIndex::kNoMapping;
		}

		return static_cast<ButtonArtIndex>(keyCode);
	}

	ButtonArtIndex ButtonArt::GetFrameIndexForMouseButton(uint16_t keyCode)
	{
		// Mouse button frames are stored in the same order as the enum, starting at frame 256.

		if (keyCode > MouseButton::kWheelDown) {
			return ButtonArtIndex::kNoMapping;
		}

		constexpr uint16_t MOUSE_BASE = static_cast<uint16_t>(ButtonArtIndex::MOUSE_BASE);
		return static_cast<ButtonArtIndex>(MOUSE_BASE + keyCode);
	}

	ButtonArtIndex ButtonArt::GetFrameIndexForGamepadInput(uint16_t keyCode)
	{
		const auto& it = _gamepadMap.find(keyCode);
		return it != _gamepadMap.end() ? it->second : ButtonArtIndex::kNoMapping;
	}

	void ButtonArt::Initialize()
	{
		if (_initialized) {
			return;
		}

		_initialized = true;

		InitializeKeyboard();
		InitializeMouse();
		InitializeGamepad();
	}

	void ButtonArt::InitializeKeyboard()
	{
		const auto AddKeyRange = [](uint16_t first, uint16_t last) noexcept {
			for (uint16_t i = first; i <= last; i++) {
				_keySet.set(i);
			}
		};

		AddKeyRange(KeyboardKey::kEscape, KeyboardKey::kKP_Decimal);
		AddKeyRange(KeyboardKey::kF11, KeyboardKey::kF12);
		AddKeyRange(KeyboardKey::kKP_Enter, KeyboardKey::kRightControl);
		AddKeyRange(KeyboardKey::kKP_Divide, KeyboardKey::kKP_Divide);
		AddKeyRange(KeyboardKey::kPrintScreen, KeyboardKey::kRightAlt);
		AddKeyRange(KeyboardKey::kPause, KeyboardKey::kPause);
		AddKeyRange(KeyboardKey::kHome, KeyboardKey::kPageUp);
		AddKeyRange(KeyboardKey::kLeft, KeyboardKey::kLeft);
		AddKeyRange(KeyboardKey::kRight, KeyboardKey::kRight);
		AddKeyRange(KeyboardKey::kEnd, KeyboardKey::kDelete);
	}

	void ButtonArt::InitializeMouse()
	{
		// Nothing to do here.
	}

	void ButtonArt::InitializeGamepad()
	{
		_gamepadMap[GamepadInput::kUp] = ButtonArtIndex::kGamepadUp;
		_gamepadMap[GamepadInput::kDown] = ButtonArtIndex::kGamepadDown;
		_gamepadMap[GamepadInput::kLeft] = ButtonArtIndex::kGamepadLeft;
		_gamepadMap[GamepadInput::kRight] = ButtonArtIndex::kGamepadRight;
		_gamepadMap[GamepadInput::kStart] = ButtonArtIndex::kGamepadStart;
		_gamepadMap[GamepadInput::kBack] = ButtonArtIndex::kGamepadBack;
		_gamepadMap[GamepadInput::kLeftThumb] = ButtonArtIndex::kGamepadLeftStick;
		_gamepadMap[GamepadInput::kRightThumb] = ButtonArtIndex::kGamepadRightStick;
		_gamepadMap[GamepadInput::kLeftShoulder] = ButtonArtIndex::kGamepadLeftButton;
		_gamepadMap[GamepadInput::kRightShoulder] = ButtonArtIndex::kGamepadRightButton;
		_gamepadMap[GamepadInput::kA] = ButtonArtIndex::kGamepadA;
		_gamepadMap[GamepadInput::kB] = ButtonArtIndex::kGamepadB;
		_gamepadMap[GamepadInput::kX] = ButtonArtIndex::kGamepadX;
		_gamepadMap[GamepadInput::kY] = ButtonArtIndex::kGamepadY;
		_gamepadMap[GamepadInput::kLeftTrigger] = ButtonArtIndex::kGamepadLeftTrigger;
		_gamepadMap[GamepadInput::kRightTrigger] = ButtonArtIndex::kGamepadRightTrigger;
	}
}
