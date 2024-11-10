#pragma once

namespace QuickLoot::Input
{
	using KeyboardKey = RE::BSWin32KeyboardDevice::Key;
	using MouseButton = RE::BSWin32MouseDevice::Key;
	using GamepadInput = RE::BSWin32GamepadDevice::Key;

	// Frame indices as provided by buttonArt.swf
	enum class ButtonArtIndex : uint16_t
	{
		kNone = 0,

		kKeyboardEscape = KeyboardKey::kEscape,
		kKeyboardNum1 = KeyboardKey::kNum1,
		kKeyboardNum2 = KeyboardKey::kNum2,
		kKeyboardNum3 = KeyboardKey::kNum3,
		kKeyboardNum4 = KeyboardKey::kNum4,
		kKeyboardNum5 = KeyboardKey::kNum5,
		kKeyboardNum6 = KeyboardKey::kNum6,
		kKeyboardNum7 = KeyboardKey::kNum7,
		kKeyboardNum8 = KeyboardKey::kNum8,
		kKeyboardNum9 = KeyboardKey::kNum9,
		kKeyboardNum0 = KeyboardKey::kNum0,
		kKeyboardMinus = KeyboardKey::kMinus,
		kKeyboardEquals = KeyboardKey::kEquals,
		kKeyboardBackspace = KeyboardKey::kBackspace,
		kKeyboardTab = KeyboardKey::kTab,
		kKeyboardQ = KeyboardKey::kQ,
		kKeyboardW = KeyboardKey::kW,
		kKeyboardE = KeyboardKey::kE,
		kKeyboardR = KeyboardKey::kR,
		kKeyboardT = KeyboardKey::kT,
		kKeyboardY = KeyboardKey::kY,
		kKeyboardU = KeyboardKey::kU,
		kKeyboardI = KeyboardKey::kI,
		kKeyboardO = KeyboardKey::kO,
		kKeyboardP = KeyboardKey::kP,
		kKeyboardBracketLeft = KeyboardKey::kBracketLeft,
		kKeyboardBracketRight = KeyboardKey::kBracketRight,
		kKeyboardEnter = KeyboardKey::kEnter,
		kKeyboardLeftControl = KeyboardKey::kLeftControl,
		kKeyboardA = KeyboardKey::kA,
		kKeyboardS = KeyboardKey::kS,
		kKeyboardD = KeyboardKey::kD,
		kKeyboardF = KeyboardKey::kF,
		kKeyboardG = KeyboardKey::kG,
		kKeyboardH = KeyboardKey::kH,
		kKeyboardJ = KeyboardKey::kJ,
		kKeyboardK = KeyboardKey::kK,
		kKeyboardL = KeyboardKey::kL,
		kKeyboardSemicolon = KeyboardKey::kSemicolon,
		kKeyboardApostrophe = KeyboardKey::kApostrophe,
		kKeyboardTilde = KeyboardKey::kTilde,
		kKeyboardLeftShift = KeyboardKey::kLeftShift,
		kKeyboardBackslash = KeyboardKey::kBackslash,
		kKeyboardZ = KeyboardKey::kZ,
		kKeyboardX = KeyboardKey::kX,
		kKeyboardC = KeyboardKey::kC,
		kKeyboardV = KeyboardKey::kV,
		kKeyboardB = KeyboardKey::kB,
		kKeyboardN = KeyboardKey::kN,
		kKeyboardM = KeyboardKey::kM,
		kKeyboardComma = KeyboardKey::kComma,
		kKeyboardPeriod = KeyboardKey::kPeriod,
		kKeyboardSlash = KeyboardKey::kSlash,
		kKeyboardRightShift = KeyboardKey::kRightShift,
		kKeyboardKP_Multiply = KeyboardKey::kKP_Multiply,
		kKeyboardLeftAlt = KeyboardKey::kLeftAlt,
		kKeyboardSpacebar = KeyboardKey::kSpacebar,
		kKeyboardCapsLock = KeyboardKey::kCapsLock,
		kKeyboardF1 = KeyboardKey::kF1,
		kKeyboardF2 = KeyboardKey::kF2,
		kKeyboardF3 = KeyboardKey::kF3,
		kKeyboardF4 = KeyboardKey::kF4,
		kKeyboardF5 = KeyboardKey::kF5,
		kKeyboardF6 = KeyboardKey::kF6,
		kKeyboardF7 = KeyboardKey::kF7,
		kKeyboardF8 = KeyboardKey::kF8,
		kKeyboardF9 = KeyboardKey::kF9,
		kKeyboardF10 = KeyboardKey::kF10,
		kKeyboardNumLock = KeyboardKey::kNumLock,
		kKeyboardScrollLock = KeyboardKey::kScrollLock,
		kKeyboardKP_7 = KeyboardKey::kKP_7,
		kKeyboardKP_8 = KeyboardKey::kKP_8,
		kKeyboardKP_9 = KeyboardKey::kKP_9,
		kKeyboardKP_Subtract = KeyboardKey::kKP_Subtract,
		kKeyboardKP_4 = KeyboardKey::kKP_4,
		kKeyboardKP_5 = KeyboardKey::kKP_5,
		kKeyboardKP_6 = KeyboardKey::kKP_6,
		kKeyboardKP_Plus = KeyboardKey::kKP_Plus,
		kKeyboardKP_1 = KeyboardKey::kKP_1,
		kKeyboardKP_2 = KeyboardKey::kKP_2,
		kKeyboardKP_3 = KeyboardKey::kKP_3,
		kKeyboardKP_0 = KeyboardKey::kKP_0,
		kKeyboardKP_Decimal = KeyboardKey::kKP_Decimal,

		kKeyboardF11 = KeyboardKey::kF11,
		kKeyboardF12 = KeyboardKey::kF12,
		kKeyboardKP_Enter = KeyboardKey::kKP_Enter,
		kKeyboardRightControl = KeyboardKey::kRightControl,
		kKeyboardKP_Divide = KeyboardKey::kKP_Divide,
		kKeyboardPrintScreen = KeyboardKey::kPrintScreen,
		kKeyboardRightAlt = KeyboardKey::kRightAlt,
		kKeyboardPause = KeyboardKey::kPause,
		kKeyboardHome = KeyboardKey::kHome,
		kKeyboardUp = KeyboardKey::kUp,
		kKeyboardPageUp = KeyboardKey::kPageUp,
		kKeyboardLeft = KeyboardKey::kLeft,
		kKeyboardRight = KeyboardKey::kRight,
		kKeyboardEnd = KeyboardKey::kEnd,
		kKeyboardDown = KeyboardKey::kDown,
		kKeyboardPageDown = KeyboardKey::kPageDown,
		kKeyboardInsert = KeyboardKey::kInsert,
		kKeyboardDelete = KeyboardKey::kDelete,

		MOUSE_BASE = 256,
		kMouseLeftButton = MOUSE_BASE + 0,
		kMouseRightButton = MOUSE_BASE + 1,
		kMouseMiddleButton = MOUSE_BASE + 2,
		kMouseButton3 = MOUSE_BASE + 3,
		kMouseButton4 = MOUSE_BASE + 4,
		kMouseButton5 = MOUSE_BASE + 5,
		kMouseButton6 = MOUSE_BASE + 6,
		kMouseButton7 = MOUSE_BASE + 7,
		kMouseWheelUp = MOUSE_BASE + 8,
		kMouseWheelDown = MOUSE_BASE + 9,

		GAMEPAD_BASE = 266,
		kGamepadUp = GAMEPAD_BASE + 0,
		kGamepadDown = GAMEPAD_BASE + 1,
		kGamepadLeft = GAMEPAD_BASE + 2,
		kGamepadRight = GAMEPAD_BASE + 3,
		kGamepadStart = GAMEPAD_BASE + 4,
		kGamepadBack = GAMEPAD_BASE + 5,
		kGamepadLeftStick = GAMEPAD_BASE + 6,
		kGamepadRightStick = GAMEPAD_BASE + 7,
		kGamepadLeftButton = GAMEPAD_BASE + 8,
		kGamepadRightButton = GAMEPAD_BASE + 9,
		kGamepadA = GAMEPAD_BASE + 10,
		kGamepadB = GAMEPAD_BASE + 11,
		kGamepadX = GAMEPAD_BASE + 12,
		kGamepadY = GAMEPAD_BASE + 13,
		kGamepadLeftTrigger = GAMEPAD_BASE + 14,
		kGamepadRightTrigger = GAMEPAD_BASE + 15,

		kNoMapping = 282,
	};
}
