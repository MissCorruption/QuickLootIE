#include "Input/InputManager.h"

#include "LootMenuManager.h"
#include "Util/HookUtil.h"

namespace QuickLoot::Input
{
	struct PatchSE : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 67254;
		static constexpr uint64_t functionStart = 0xC120B0;
		static constexpr uint64_t patchStart = 0xC1238A;
		static constexpr uint64_t patchEnd = 0xC12397;

		explicit PatchSE()
		{
			pop(r15);
			pop(r14);
			pop(r13);
			pop(r12);
			pop(rdi);
			pop(rsi);
			pop(rbx);
			pop(rbp);

			mov(rax, reinterpret_cast<uintptr_t>(InputManager::UpdateMappings));
			jmp(rax);
		}
	};

	struct PatchAE : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 68554;
		static constexpr uint64_t functionStart = 0xCD5A80;
		static constexpr uint64_t patchStart = 0xCD5F7B;
		static constexpr uint64_t patchEnd = 0xCD5F85;

		explicit PatchAE()
		{
			pop(r15);
			pop(r14);
			pop(r13);
			pop(r12);
			pop(rdi);

			mov(rax, reinterpret_cast<uintptr_t>(InputManager::UpdateMappings));
			jmp(rax);
		}
	};

	struct PatchVR : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 67254;
		static constexpr uint64_t functionStart = 0xC4EA50;
		static constexpr uint64_t patchStart = 0xC4ED2A;
		static constexpr uint64_t patchEnd = 0xC4ED37;

		explicit PatchVR()
		{
			pop(r15);
			pop(r14);
			pop(r13);
			pop(r12);
			pop(rdi);
			pop(rsi);
			pop(rbx);
			pop(rbp);

			mov(rax, reinterpret_cast<uintptr_t>(InputManager::UpdateMappings));
			jmp(rax);
		}
	};

	void InputManager::Install()
	{
		// We patch a tail call to InputManager::UpdateMappings into
		// ControlMap::RefreshLinkedMappings in order to perform our own post-processing logic.

		switch (REL::Module::GetRuntime()) {
		case REL::Module::Runtime::AE:
			Util::HookUtil::WritePatch<PatchAE>();
			break;

		case REL::Module::Runtime::SE:
			Util::HookUtil::WritePatch<PatchSE>();
			break;

		case REL::Module::Runtime::VR:
			Util::HookUtil::WritePatch<PatchVR>();
			break;

		default:
			logger::error("Invalid runtime");
			break;
		}

		UpdateMappings();

		_grabDelaySetting = RE::GetINISetting("fZKeyDelay:Controls");

		logger::info("Installed {}", typeid(InputManager).name());
	}

	// The way the input suppression mechanism works is that all user event mappings conflicting with QuickLoot are
	// added to a new user event group QUICKLOOT_EVENT_GROUP_FLAG. This can then be toggled via ControlMap::ToggleControls.
	void InputManager::UpdateMappings()
	{
		ReloadKeybindings();

		logger::info("Updating event mappings");

		std::set<ControlGroup> disabledGroups{};

		// Clear the quickloot flag of all valid mappings
		WalkMappings([&](UserEventMapping& mapping, DeviceType) {
			if (mapping.userEventGroupFlag.none(UEFlag::kInvalid)) {
				mapping.userEventGroupFlag.reset(QUICKLOOT_EVENT_GROUP_FLAG);
			}
		});

		// Find disabled keybinding groups
		WalkMappings([&](const UserEventMapping& mapping, DeviceType deviceType) {
			const auto* conflicting = FindConflictingKeybinding(mapping, deviceType);
			if (conflicting && conflicting->group & ControlGroup::kOptional) {
				const auto [_, success] = disabledGroups.insert(conflicting->group.get());
				if (success) {
					logger::info("Disabling optional control group {}", conflicting->group.underlying());
				}
			}
		});

		// Add mappings to the quickloot user event group
		WalkMappings([&](UserEventMapping& mapping, DeviceType deviceType) {
			const auto* conflicting = FindConflictingKeybinding(mapping, deviceType);
			if (!conflicting || disabledGroups.contains(conflicting->group.get())) {
				return;
			}

			if (mapping.userEventGroupFlag.all(UEFlag::kInvalid)) {
				mapping.userEventGroupFlag = UEFlag::kNone;
			}

			mapping.userEventGroupFlag.set(QUICKLOOT_EVENT_GROUP_FLAG);

			logger::debug("Added mapping to the QuickLoot user event group: {} (device {}, key code {})", mapping.eventID, static_cast<int>(deviceType), mapping.inputKey);
		});

		LootMenuManager::RequestRefresh(RefreshFlags::kButtonBar);
	}

	void InputManager::BlockConflictingInputs()
	{
		RE::ControlMap::GetSingleton()->ToggleControls(QUICKLOOT_EVENT_GROUP_FLAG, false);

		if (REL::Module::IsVR()) {
			const auto playerControls = RE::PlayerControls::GetSingleton();
			playerControls->sneakHandler->SetInputEventHandlingEnabled(false);
			playerControls->jumpHandler->SetInputEventHandlingEnabled(false);
		}
	}

	void InputManager::UnblockConflictingInputs()
	{
		RE::ControlMap::GetSingleton()->ToggleControls(QUICKLOOT_EVENT_GROUP_FLAG, true);

		if (REL::Module::IsVR()) {
			const auto playerControls = RE::PlayerControls::GetSingleton();
			playerControls->sneakHandler->SetInputEventHandlingEnabled(true);
			playerControls->jumpHandler->SetInputEventHandlingEnabled(true);
		}
	}

	void InputManager::HandleButtonEvent(const RE::ButtonEvent* event)
	{
		UpdateModifierKeys(event);

		Keybinding* keybinding = FindMatchingKeybinding(event);

		if (!keybinding) {
			return;
		}

		if (HandleGrab(event, keybinding)) {
			return;
		}

		if (event->IsDown()) {
			keybinding->nextRetriggerTime = 0.0f;
			TriggerKeybinding(keybinding);
			return;
		}

		HandleRetrigger(event, keybinding);
	}

	void InputManager::HandleThumbstickEvent(const RE::ThumbstickEvent* event)
	{
		if (!REL::Module::IsVR()) {
			return;
		}

		if (!event->IsMainHand()) {
			return;
		}

		//logger::debug("Thumbstick event: {}/{}, {}", event->xValue, event->yValue, event->IsMainHand());

		static float lastYValue = 0;
		static float startTime = 0;

		constexpr float pressThreshold = 0.5f;
		constexpr float releaseThreshold = 0.2f;

		bool wasUpPressed = startTime != 0 && lastYValue > 0;
		bool wasDownPressed = startTime != 0 && lastYValue < 0;

		bool isUpPressed = event->yValue > (wasUpPressed ? releaseThreshold : pressThreshold);
		bool isDownPressed = event->yValue < -(wasUpPressed ? releaseThreshold : pressThreshold);

		float now = static_cast<float>(GetTickCount()) * 0.001f;
		lastYValue = event->yValue;

		if (isUpPressed) {
			if (wasUpPressed) {
				SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickUp, 1.0f, now - startTime);
			} else {
				startTime = now;
				SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickUp, 1.0f, 0.0f);
			}
		} else if (wasUpPressed) {
			SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickUp, 0.0f, now - startTime);
			startTime = 0;
		}

		if (isDownPressed) {
			if (wasDownPressed) {
				SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickDown, 1.0f, now - startTime);
			} else {
				startTime = now;
				SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickDown, 1.0f, 0.0f);
			}
		} else if (wasDownPressed) {
			SendFakeButtonEvent(event->device.get(), VRInput::kMainThumbStickDown, 0.0f, now - startTime);
			startTime = 0;
		}
	}

	bool QUsingGamepad(RE::BSInputDeviceManager* _this)
	{
		using func_t = decltype(&QUsingGamepad);
		REL::Relocation<func_t> func{ RELOCATION_ID(67320, 443396) };
		return func(_this);
	}

	std::vector<Keybinding> InputManager::GetButtonBarKeybindings()
	{
		std::vector<Keybinding> filtered{};

		bool isGamepad = QUsingGamepad(RE::BSInputDeviceManager::GetSingleton());

		std::ranges::copy_if(_keybindings, std::back_inserter(filtered), [=](const Keybinding& keybinding) {
			return keybinding.group == ControlGroup::kButtonBar &&
			       (keybinding.deviceType == DeviceType::kGamepad) == isGamepad &&
			       (keybinding.modifiers == ModifierKeys::kIgnore ||
					   keybinding.modifiers == (_currentModifiers & _usedModifiers));
		});

		return filtered;
	}

	void InputManager::ReloadKeybindings()
	{
		_keybindings.clear();

		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelUp, ModifierKeys::kNone, QuickLootAction::kScrollUp, false);
		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelDown, ModifierKeys::kNone, QuickLootAction::kScrollDown, false);
		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelUp, ModifierKeys::kShift, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelDown, ModifierKeys::kShift, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kUp, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kDown, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kLeft, ModifierKeys::kIgnore, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kRight, ModifierKeys::kIgnore, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_8, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_2, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_4, ModifierKeys::kIgnore, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_6, ModifierKeys::kIgnore, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceType::kKeyboard, KeyboardKey::kPageUp, ModifierKeys::kIgnore, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceType::kKeyboard, KeyboardKey::kPageDown, ModifierKeys::kIgnore, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kUp, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kDown, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kLeft, ModifierKeys::kIgnore, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kRight, ModifierKeys::kIgnore, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kOculusPrimary, VRInput::kMainThumbStickUp, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kOculusPrimary, VRInput::kMainThumbStickDown, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kVivePrimary, VRInput::kMainThumbStickUp, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kVivePrimary, VRInput::kMainThumbStickDown, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kWMRPrimary, VRInput::kMainThumbStickUp, ModifierKeys::kIgnore, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceType::kWMRPrimary, VRInput::kMainThumbStickDown, ModifierKeys::kIgnore, QuickLootAction::kScrollDown, true);

		_keybindings.append_range(Settings::GetKeybindings());

		_usedModifiers = ModifierKeys::kNone;

		for (const auto& keybinding : _keybindings) {
			if (keybinding.modifiers != ModifierKeys::kIgnore) {
				_usedModifiers |= keybinding.modifiers;
			}
		}
	}

	Keybinding* InputManager::FindConflictingKeybinding(const UserEventMapping& mapping, DeviceType deviceType)
	{
		const auto it = std::ranges::find_if(_keybindings, [&](const Keybinding& keybinding) {
			return keybinding.deviceType == deviceType &&
			       keybinding.inputKey == mapping.inputKey &&
			       !keybinding.global;
		});

		return it != _keybindings.end() ? &*it : nullptr;
	}

	Keybinding* InputManager::FindMatchingKeybinding(const RE::ButtonEvent* event)
	{
		const auto deviceType = event->GetDevice();
		auto inputKey = event->GetIDCode();

		if (deviceType == RE::INPUT_DEVICE::kGamepad &&
			RE::ControlMap::GetSingleton()->GetGamePadType() == RE::PC_GAMEPAD_TYPE::kOrbis) {
			inputKey = SKSE::InputMap::ScePadOffsetToXInput(inputKey);
		}

		const auto it = std::ranges::find_if(_keybindings, [&](const Keybinding& keybinding) {
			return keybinding.deviceType == deviceType &&
			       keybinding.inputKey == inputKey &&
			       (keybinding.modifiers == ModifierKeys::kIgnore ||
					   keybinding.modifiers == (_currentModifiers & _usedModifiers)) &&
			       (keybinding.global || LootMenuManager::IsShowing());
		});

		return it != _keybindings.end() ? &*it : nullptr;
	}

	void InputManager::SendFakeButtonEvent(DeviceType device, int idCode, float value, float heldDownSecs)
	{
		logger::debug("Fake button event: device {}, key {} {} for {}s", static_cast<uint32_t>(device), idCode, value > 0 ? "down" : "up", heldDownSecs);

		static auto fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kNone, "", 0, 0, 0);

		fakeEvent->eventType = RE::INPUT_EVENT_TYPE::kButton;
		fakeEvent->device = device;
		fakeEvent->idCode = idCode;
		fakeEvent->userEvent = "";
		fakeEvent->GetRuntimeData().value = value;
		fakeEvent->GetRuntimeData().heldDownSecs = heldDownSecs;

		HandleButtonEvent(fakeEvent);
	}

	void InputManager::UpdateModifierKeys(const RE::ButtonEvent* event)
	{
		const auto deviceType = event->GetDevice();
		const auto inputKey = static_cast<KeyboardKey>(event->GetIDCode());

		if (deviceType != RE::INPUT_DEVICE::kKeyboard) {
			return;
		}

		switch (inputKey) {
		case KeyboardKey::kLeftShift:
		case KeyboardKey::kRightShift:
		case KeyboardKey::kLeftControl:
		case KeyboardKey::kRightControl:
		case KeyboardKey::kLeftAlt:
		case KeyboardKey::kRightAlt:
			break;

		default:
			return;
		}

		UpdateModifierKeys();
	}

	void InputManager::UpdateModifierKeys()
	{
		if (REL::Module::IsVR()) {
			return;
		}

		const auto oldModifiers = _currentModifiers;
		_currentModifiers = ModifierKeys::kNone;

		// We have to upcast this to BSKeyboardDevice because BSWin32KeyboardDevice::IsPressed is not implemented in CLib.
		const auto* keyboard = reinterpret_cast<RE::BSKeyboardDevice*>(RE::BSInputDeviceManager::GetSingleton()->GetKeyboard());

		if (keyboard->IsPressed(KeyboardKey::kLeftShift) || keyboard->IsPressed(KeyboardKey::kRightShift)) {
			_currentModifiers |= ModifierKeys::kShift;
		}
		if (keyboard->IsPressed(KeyboardKey::kLeftControl) || keyboard->IsPressed(KeyboardKey::kRightControl)) {
			_currentModifiers |= ModifierKeys::kControl;
		}
		if (keyboard->IsPressed(KeyboardKey::kLeftAlt) || keyboard->IsPressed(KeyboardKey::kRightAlt)) {
			_currentModifiers |= ModifierKeys::kAlt;
		}

		if (_currentModifiers != oldModifiers) {
			// The button bar needs to be updated when the pressed modifier keys change.
			LootMenuManager::RequestRefresh(RefreshFlags::kButtonBar);
		}
	}

	bool InputManager::HandleGrab(const RE::ButtonEvent* event, const Keybinding* keybinding)
	{
		const auto activateKey = RE::ControlMap::GetSingleton()->GetMappedKey("Activate", event->GetDevice());

		if (event->GetIDCode() != activateKey) {
			return false;
		}

		if (event->IsDown()) {
			_triggerOnActivateRelease = LootMenuManager::IsShowing();
			return true;
		}

		// For the activate key, the up event is used to trigger the action.
		if (!event->IsPressed() && _triggerOnActivateRelease) {
			TriggerKeybinding(keybinding);
			_triggerOnActivateRelease = false;
			return true;
		}

		if (!event->IsHeld() || event->HeldDuration() < _grabDelaySetting->GetFloat()) {
			return true;
		}

		if (TryGrab()) {
			_triggerOnActivateRelease = false;
		}

		return true;
	}

	bool InputManager::TryGrab()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		player->StartGrabObject();
		if (!player->IsGrabbing()) {
			return false;
		}

		if (auto activateHandler = RE::PlayerControls::GetSingleton()->GetActivateHandler()) {
			activateHandler->SetHeldButtonActionSuccess(true);
		}

		LootMenuManager::RequestHide();
		return true;
	}

	void InputManager::TriggerKeybinding(const Keybinding* keybinding)
	{
		LootMenuManager::OnInputAction(keybinding->action);
	}

	void InputManager::HandleRetrigger(const RE::ButtonEvent* event, Keybinding* keybinding)
	{
		if (!keybinding->retrigger) {
			return;
		}

		if (event->IsUp()) {
			keybinding->nextRetriggerTime = 0.0f;
			return;
		}

		const auto holdTime = event->HeldDuration();

		constexpr auto initialDelay = 0.5f;
		constexpr auto subsequentDelay = 0.05f;

		if (keybinding->nextRetriggerTime < initialDelay) {
			keybinding->nextRetriggerTime = initialDelay;
		}

		if (holdTime >= keybinding->nextRetriggerTime) {
			keybinding->nextRetriggerTime += subsequentDelay;
			TriggerKeybinding(keybinding);
		}
	}

	void InputManager::WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts)
	{
		const auto controlMap = RE::ControlMap::GetSingleton();
		if (!controlMap) {
			logger::error("Unable to access control map");
			return;
		}

		int contextCount = RE::UserEvents::INPUT_CONTEXT_ID::kAETotal;
		if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_1130) == std::strong_ordering::less) {
			contextCount = 17;
		}

		if (!allContexts) {
			// Only walk kGameplay input context
			contextCount = 1;
		}

		for (int contextId = 0; contextId < contextCount; ++contextId) {
			const auto context = controlMap->controlMap[contextId];
			if (!context)
				continue;

			for (int deviceType = 0; deviceType < DeviceType::kFlatTotal; ++deviceType) {
				for (auto& userMapping : context->deviceMappings[deviceType]) {
					functor(userMapping, static_cast<DeviceType>(deviceType));
				}
			}
		}
	}
}
