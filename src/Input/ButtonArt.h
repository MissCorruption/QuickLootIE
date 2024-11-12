#pragma once
#include <map>

#include "Input/ButtonArtIndex.h"

namespace QuickLoot::Input
{
	class ButtonArt
	{
	public:
		ButtonArt() = delete;
		~ButtonArt() = delete;
		ButtonArt(ButtonArt&&) = delete;
		ButtonArt(const ButtonArt&) = delete;
		ButtonArt& operator=(ButtonArt&&) = delete;
		ButtonArt& operator=(const ButtonArt&) = delete;

		static ButtonArtIndex GetFrameIndexForDeviceKey(DeviceType deviceType, uint16_t keyCode);
		static ButtonArtIndex GetFrameIndexForEvent(const std::string_view& event);

	private:
		static inline bool _initialized = false;
		static inline std::bitset<KeyboardKey::kDelete + 1> _keySet{};
		static inline std::map<uint16_t, ButtonArtIndex> _gamepadMap{};

		static ButtonArtIndex GetFrameIndexForKeyboardKey(uint16_t keyCode);
		static ButtonArtIndex GetFrameIndexForMouseButton(uint16_t keyCode);
		static ButtonArtIndex GetFrameIndexForGamepadInput(uint16_t keyCode);

		static void Initialize();
		static void InitializeKeyboard();
		static void InitializeMouse();
		static void InitializeGamepad();
	};
}
