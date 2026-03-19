#include "Input/InputManager.h"

#include "ButtonArtIndex.h"
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
			const auto conflicts = conflicting && !disabledGroups.contains(conflicting->group.get());

			if (!conflicts && mapping.eventID != "Activate") {
				return;
			}

			if (mapping.userEventGroupFlag.all(UEFlag::kInvalid)) {
				mapping.userEventGroupFlag = UEFlag::kNone;
			}

			mapping.userEventGroupFlag.set(QUICKLOOT_EVENT_GROUP_FLAG);

			logger::debug("Added mapping to the QuickLoot user event group: {} (device {}, key code {})",
				std::string_view(mapping.eventID), static_cast<int>(deviceType), mapping.inputKey);
		});

		LootMenuManager::RequestRefresh(RefreshFlags::kButtonBar);
	}

	void InputManager::BlockConflictingInputs()
	{
		RE::ControlMap::GetSingleton()->ToggleControls(QUICKLOOT_EVENT_GROUP_FLAG, false, false);

		if (REL::Module::IsVR()) {
			const auto playerControls = RE::PlayerControls::GetSingleton();
			playerControls->sneakHandler->SetInputEventHandlingEnabled(false);
			playerControls->jumpHandler->SetInputEventHandlingEnabled(false);
		}
	}

	void InputManager::UnblockConflictingInputs()
	{
		RE::ControlMap::GetSingleton()->ToggleControls(QUICKLOOT_EVENT_GROUP_FLAG, true, false);

		if (REL::Module::IsVR()) {
			const auto playerControls = RE::PlayerControls::GetSingleton();
			playerControls->sneakHandler->SetInputEventHandlingEnabled(true);
			playerControls->jumpHandler->SetInputEventHandlingEnabled(true);
		}
	}

	void InputManager::HandleButtonEvent(const RE::ButtonEvent* event)
	{
		const DeviceKey eventKey = {
			.deviceType = event->GetDevice(),
			.keyCode = event->GetIDCode(),
		};

		if (_allModifierKeys.contains(eventKey)) {
			UpdateModifierStates();
		}

		if (!_allInputKeys.contains(eventKey)) {
			return;
		}

		Keybinding* keybinding = FindSatisfiedKeybinding(event);

		if (HandleGrab(event, keybinding)) {
			return;
		}

		if (!keybinding) {
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
		bool isDownPressed = event->yValue < -(wasDownPressed ? releaseThreshold : pressThreshold);

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
		uint64_t aeId = 68622;
		if (REL::Module::get().version() >= REL::Version(1, 6, 1130, 0)) aeId = 443396;
		if (REL::Module::get().version() >= REL::Version(1, 6, 1179, 0)) aeId = 510926;

		using func_t = decltype(&QUsingGamepad);
		REL::Relocation<func_t> func{ RELOCATION_ID(67320, aeId) };
		return func(_this);
	}

	std::vector<Keybinding> InputManager::GetButtonBarKeybindings()
	{
		std::vector<Keybinding> filtered{};

		if (REL::Module::IsVR()) {
			filtered.push_back(Keybinding{ .action = QuickLootAction::kTake, .buttonArtOverride = ButtonArtIndex::kOculusA });
			filtered.push_back(Keybinding{ .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kOculusAHold });
		} else {
			const bool isGamepad = QUsingGamepad(RE::BSInputDeviceManager::GetSingleton());

			std::ranges::copy_if(_keybindings, std::back_inserter(filtered), [=](const Keybinding& keybinding) {
				return keybinding.group == ControlGroup::kButtonBar &&
				       keybinding.isModifierSatisfied &&
				       (keybinding.inputKey.deviceType == DeviceType::kGamepad) == isGamepad;
			});
		}

		return filtered;
	}

	void InputManager::ReloadKeybindings()
	{
		_keybindings.clear();

		constexpr std::optional<DeviceKey> none = {};
		constexpr std::optional<DeviceKey> shift = { DeviceKey::Get(KeyboardKey::kLeftShift) };

		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceKey::Get(MouseButton::kWheelUp), none, QuickLootAction::kScrollUp, false);
		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceKey::Get(MouseButton::kWheelDown), none, QuickLootAction::kScrollDown, false);
		//_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceKey::Get(MouseButton::kWheelUp), shift, QuickLootAction::kPrevPage, false);
		//_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceKey::Get(MouseButton::kWheelDown), shift, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceKey::Get(KeyboardKey::kUp), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceKey::Get(KeyboardKey::kDown), none, QuickLootAction::kScrollDown, true);
		//_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceKey::Get(KeyboardKey::kLeft), none, QuickLootAction::kPrevPage, false);
		//_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceKey::Get(KeyboardKey::kRight), none, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceKey::Get(KeyboardKey::kKP_8), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceKey::Get(KeyboardKey::kKP_2), none, QuickLootAction::kScrollDown, true);
		//_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceKey::Get(KeyboardKey::kKP_4), none, QuickLootAction::kPrevPage, false);
		//_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceKey::Get(KeyboardKey::kKP_6), none, QuickLootAction::kNextPage, false);

		//_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceKey::Get(KeyboardKey::kPageUp), none, QuickLootAction::kPrevPage, false);
		//_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceKey::Get(KeyboardKey::kPageDown), none, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kDpad, DeviceKey::Get(GamepadInput::kUp), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceKey::Get(GamepadInput::kDown), none, QuickLootAction::kScrollDown, true);
		//_keybindings.emplace_back(ControlGroup::kDpad, DeviceKey::Get(GamepadInput::kLeft), none, QuickLootAction::kPrevPage, false);
		//_keybindings.emplace_back(ControlGroup::kDpad, DeviceKey::Get(GamepadInput::kRight), none, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kOculusPrimary, VRInput::kMainThumbStickUp), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kOculusPrimary, VRInput::kMainThumbStickDown), none, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kVivePrimary, VRInput::kMainThumbStickUp), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kVivePrimary, VRInput::kMainThumbStickDown), none, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kWMRPrimary, VRInput::kMainThumbStickUp), none, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kVrScroll, DeviceKey::Get(DeviceType::kWMRPrimary, VRInput::kMainThumbStickDown), none, QuickLootAction::kScrollDown, true);

		_keybindings.append_range(Settings::GetKeybindings());

		_allInputKeys.clear();
		_allModifierKeys.clear();

		for (size_t a = 0; a < _keybindings.size(); ++a) {
			auto& keybindingA = _keybindings[a];

			_allInputKeys.insert(keybindingA.inputKey);

			if (keybindingA.modifierKey) {
				_allModifierKeys.insert(*keybindingA.modifierKey);
			}
		}

		UpdateModifierStates();
	}

	Keybinding* InputManager::FindConflictingKeybinding(const UserEventMapping& mapping, DeviceType deviceType)
	{
		const auto it = std::ranges::find_if(_keybindings, [&](const Keybinding& keybinding) {
			return keybinding.inputKey.deviceType == deviceType &&
			       keybinding.inputKey.keyCode == mapping.inputKey &&
			       !keybinding.global;
		});

		return it != _keybindings.end() ? &*it : nullptr;
	}

	Keybinding* InputManager::FindSatisfiedKeybinding(const RE::ButtonEvent* event)
	{
		const auto deviceType = event->GetDevice();
		auto inputKey = event->GetIDCode();

		if (deviceType == RE::INPUT_DEVICE::kGamepad &&
			RE::ControlMap::GetSingleton()->GetGamePadType() == RE::PC_GAMEPAD_TYPE::kOrbis) {
			inputKey = SKSE::InputMap::ScePadOffsetToXInput(inputKey);
		}

		const auto it = std::ranges::find_if(_keybindings, [&](const Keybinding& keybinding) {
			return (keybinding.global || LootMenuManager::IsShowing()) &&
			       keybinding.inputKey.deviceType == deviceType &&
			       keybinding.inputKey.keyCode == inputKey &&
			       keybinding.isModifierSatisfied;
		});

		return it != _keybindings.end() ? &*it : nullptr;
	}

	bool InputManager::IsKeyPressed(DeviceKey key)
	{
		const auto device = GetInputDevice(key.deviceType);
		return device && device->IsPressed(key.keyCode);
	}

	RE::BSInputDevice* InputManager::GetInputDevice(DeviceType deviceType)
	{
		const auto deviceManager = RE::BSInputDeviceManager::GetSingleton();

		switch (deviceType) {
		case DeviceType::kKeyboard:
			return deviceManager->GetKeyboard();

		case DeviceType::kMouse:
			return deviceManager->GetMouse();

		case DeviceType::kGamepad:
			return deviceManager->GetGamepad();

		case DeviceType::kOculusPrimary:
		case DeviceType::kVivePrimary:
		case DeviceType::kWMRPrimary:
			{
				const auto rightHanded = RE::PlayerCharacter::GetSingleton()->GetVRPlayerRuntimeData().isRightHandMainHand;
				return rightHanded ? deviceManager->GetVRControllerRight() : deviceManager->GetVRControllerLeft();
			}

		case DeviceType::kOculusSecondary:
		case DeviceType::kViveSecondary:
		case DeviceType::kWMRSecondary:
			{
				const auto rightHanded = RE::PlayerCharacter::GetSingleton()->GetVRPlayerRuntimeData().isRightHandMainHand;
				return rightHanded ? deviceManager->GetVRControllerLeft() : deviceManager->GetVRControllerRight();
			}

		default:
			return nullptr;
		}
	}

	void InputManager::SendFakeButtonEvent(DeviceType device, int idCode, float value, float heldDownSecs)
	{
		//logger::debug("Fake button event: device {}, key {} {} (held for {:.2f}s)", static_cast<uint32_t>(device), idCode, value > 0 ? "down" : "up", heldDownSecs);

		static auto fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kNone, "", 0, 0, 0);

		fakeEvent->eventType = RE::INPUT_EVENT_TYPE::kButton;
		fakeEvent->device = device;
		fakeEvent->AsIDEvent()->idCode = idCode;
		fakeEvent->AsIDEvent()->userEvent = "";
		fakeEvent->GetRuntimeData().value = value;
		fakeEvent->GetRuntimeData().heldDownSecs = heldDownSecs;

		HandleButtonEvent(fakeEvent);
	}

	void InputManager::UpdateModifierStates()
	{
		std::set<DeviceKey> suppressedInputKeys{};
		bool requiresButtonBarUpdate = false;

		// check keybindings with modifier first
		for (auto& keybinding : _keybindings) {
			if (!keybinding.modifierKey) {
				continue;
			}

			const bool wasSatisfied = keybinding.isModifierSatisfied;
			const bool isSatisfied = !suppressedInputKeys.contains(keybinding.inputKey) && IsKeyPressed(*keybinding.modifierKey);
			keybinding.isModifierSatisfied = isSatisfied;

			if (isSatisfied) {
				suppressedInputKeys.insert(keybinding.inputKey);
			}

			if (isSatisfied != wasSatisfied && keybinding.group == ControlGroup::kButtonBar) {
				requiresButtonBarUpdate = true;
			}
		}

		// check keybindings without modifier
		for (auto& keybinding : _keybindings) {
			if (keybinding.modifierKey) {
				continue;
			}

			const bool wasSatisfied = keybinding.isModifierSatisfied;
			const bool isSatisfied = !suppressedInputKeys.contains(keybinding.inputKey);
			keybinding.isModifierSatisfied = isSatisfied;

			if (isSatisfied != wasSatisfied && keybinding.group == ControlGroup::kButtonBar) {
				requiresButtonBarUpdate = true;
			}
		}

		if (requiresButtonBarUpdate) {
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
		if (!event->IsPressed() && _triggerOnActivateRelease && keybinding) {
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

		if (!LootMenuManager::IsShowing()) {
			return false;
		}

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
