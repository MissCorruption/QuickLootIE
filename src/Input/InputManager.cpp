#include "Input/InputManager.h"

#include "ButtonArtIndex.h"
#include "LootMenuManager.h"
#include "Util/HookUtil.h"

#if defined(ENABLE_SKYRIM_VR)
#	include "RE/B/BSOpenVRControllerDevice.h"
#endif

namespace QuickLoot::Input
{
#if defined(ENABLE_SKYRIM_VR)
	namespace
	{
		bool IsVrGripKeyCode(std::uint32_t keyCode)
		{
			return RE::BSOpenVRControllerDevice::IsGripButton(keyCode);
		}

		bool IsVrPrimaryFamily(DeviceType deviceType)
		{
			return deviceType == DeviceType::kOculusPrimary ||
			       deviceType == DeviceType::kVivePrimary ||
			       deviceType == DeviceType::kWMRPrimary;
		}

		bool IsVrSecondaryFamily(DeviceType deviceType)
		{
			return deviceType == DeviceType::kOculusSecondary ||
			       deviceType == DeviceType::kViveSecondary ||
			       deviceType == DeviceType::kWMRSecondary;
		}

		bool VrDevicesEquivalent(DeviceType bindingDevice, DeviceType eventDevice)
		{
			if (bindingDevice == eventDevice) {
				return true;
			}

			if (IsVrPrimaryFamily(bindingDevice)) {
				return RE::BSOpenVRControllerDevice::IsPrimaryController(eventDevice);
			}

			if (IsVrSecondaryFamily(bindingDevice)) {
				return RE::BSOpenVRControllerDevice::IsSecondaryController(eventDevice);
			}

			return false;
		}

		bool VrKeyCodesEquivalent(std::uint32_t bindingKeyCode, std::uint32_t eventKeyCode)
		{
			if (bindingKeyCode == eventKeyCode) {
				return true;
			}

			return IsVrGripKeyCode(bindingKeyCode) && IsVrGripKeyCode(eventKeyCode);
		}

		bool IsRegisteredInputKey(const std::set<DeviceKey>& registeredKeys, DeviceKey eventKey)
		{
			if (registeredKeys.contains(eventKey)) {
				return true;
			}

			if (!REL::Module::IsVR()) {
				return false;
			}

			return std::ranges::any_of(registeredKeys, [&](const DeviceKey& registeredKey) {
				return VrKeyCodesEquivalent(registeredKey.keyCode, eventKey.keyCode) &&
				       VrDevicesEquivalent(registeredKey.deviceType, eventKey.deviceType);
			});
		}

		bool KeybindingMatchesEvent(const Keybinding& keybinding, DeviceType deviceType, std::uint32_t inputKey)
		{
			if (!VrKeyCodesEquivalent(keybinding.inputKey.keyCode, inputKey)) {
				return false;
			}

			return VrDevicesEquivalent(keybinding.inputKey.deviceType, deviceType);
		}

		bool IsVrTriggerKeyCode(std::uint32_t keyCode)
		{
			return keyCode == VRInput::kTrigger || keyCode == VRInput::kJoystickTrigger;
		}

		bool IsVrTakeAllButtonPress(const RE::ButtonEvent* event)
		{
			return event->IsUp();
		}

		bool IsVrControllerDevice(DeviceType deviceType)
		{
			return deviceType == DeviceType::kOculusPrimary ||
			       deviceType == DeviceType::kOculusSecondary ||
			       deviceType == DeviceType::kVivePrimary ||
			       deviceType == DeviceType::kViveSecondary ||
			       deviceType == DeviceType::kWMRPrimary ||
			       deviceType == DeviceType::kWMRSecondary;
		}
	}
#endif
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

			// ToggleControls only disables mappings tagged in the gameplay input context, but the
			// VR ActivateHandler fires the vanilla "activate" (open container) behavior independent
			// of that translation layer, so it needs to be disabled directly or "A" opens the
			// container underneath our menu instead of being treated as Take/Grab.
			if (const auto activateHandler = playerControls->GetActivateHandler()) {
				activateHandler->SetInputEventHandlingEnabled(false);
			}
		}
	}

	void InputManager::UnblockConflictingInputs()
	{
		RE::ControlMap::GetSingleton()->ToggleControls(QUICKLOOT_EVENT_GROUP_FLAG, true, false);

		if (REL::Module::IsVR()) {
			const auto playerControls = RE::PlayerControls::GetSingleton();
			playerControls->sneakHandler->SetInputEventHandlingEnabled(true);
			playerControls->jumpHandler->SetInputEventHandlingEnabled(true);

			if (const auto activateHandler = playerControls->GetActivateHandler()) {
				activateHandler->SetInputEventHandlingEnabled(true);
			}
		}
	}

	void InputManager::HandleButtonEvent(const RE::ButtonEvent* event)
	{
		const DeviceKey eventKey = {
			.deviceType = event->GetDevice(),
			.keyCode = event->GetIDCode(),
		};

#if defined(ENABLE_SKYRIM_VR)
		if (REL::Module::IsVR() && IsVrGripKeyCode(eventKey.keyCode)) {
			if (event->IsUp()) {
				_suppressTransferUntilGripRelease = false;
			} else if (event->IsDown() && RE::UI::GetSingleton()->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) {
				// Grip closes ContainerMenu via vanilla UI, then QuickLoot re-shows on the same
				// look-at before this press ends - block Transfer until grip is fully released.
				_suppressTransferUntilGripRelease = true;
			}
		}
#endif

		if (_allModifierKeys.contains(eventKey)) {
			UpdateModifierStates();
#if defined(ENABLE_SKYRIM_VR)
			if (REL::Module::IsVR() && LootMenuManager::IsShowing() && IsVrTriggerKeyCode(eventKey.keyCode)) {
				LootMenuManager::RequestRefresh(RefreshFlags::kButtonBar);
			}
#endif
		}

#if defined(ENABLE_SKYRIM_VR)
		if (!IsRegisteredInputKey(_allInputKeys, eventKey)) {
#else
		if (!_allInputKeys.contains(eventKey)) {
#endif
			return;
		}

		Keybinding* keybinding = FindSatisfiedKeybinding(event);

		if (HandleGrab(event, keybinding)) {
			return;
		}

		if (!keybinding) {
			return;
		}

#if defined(ENABLE_SKYRIM_VR)
		// Quest controllers often skip IsDown() on B/Y; accept several press edge shapes.
		if (REL::Module::IsVR() && keybinding->action == QuickLootAction::kTakeAll) {
			if (IsVrTakeAllButtonPress(event)) {
				TriggerKeybinding(keybinding);
			}
			return;
		}
#endif

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
			// Take/Use share the wand Activate button (tap). Use requires holding the trigger
			// first, matching flatrim's modifier-key Use binding.
			if (IsVrUseModifierPressed()) {
				filtered.push_back(Keybinding{ .action = QuickLootAction::kUse, .buttonArtOverride = ButtonArtIndex::kVrTriggerHold });
			} else {
				filtered.push_back(Keybinding{ .action = QuickLootAction::kTake, .buttonArtOverride = ButtonArtIndex::kOculusA });
			}

			// TakeAll/Transfer are displayed once each regardless of headset brand, even though
			// _keybindings holds a separate functional entry per VR device family (Oculus/Vive/WMR
			// all bind the same physical button) - filtering by device type there would show them
			// once per brand instead of once total.
			filtered.push_back(Keybinding{ .action = QuickLootAction::kTakeAll, .buttonArtOverride = ButtonArtIndex::kOculusB });
			filtered.push_back(Keybinding{ .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kVrGrip });
		} else {
			UpdateModifierStates();

			const bool isGamepad = QUsingGamepad(RE::BSInputDeviceManager::GetSingleton());

			const auto collectBindings = [&](bool gamepadFilter) {
				for (const auto& keybinding : _keybindings) {
					if (keybinding.group != ControlGroup::kButtonBar || !keybinding.isModifierSatisfied) {
						continue;
					}

#if defined(ENABLE_SKYRIM_VR)
					if (IsVrControllerDevice(keybinding.inputKey.deviceType)) {
						continue;
					}
#endif

					if ((keybinding.inputKey.deviceType == DeviceType::kGamepad) != gamepadFilter) {
						continue;
					}

					if (std::ranges::any_of(filtered, [&](const Keybinding& existing) {
							return existing.action == keybinding.action;
						})) {
						continue;
					}

					filtered.push_back(keybinding);
				}
			};

			collectBindings(isGamepad);
			if (filtered.empty()) {
				collectBindings(!isGamepad);
			}
			if (filtered.empty()) {
				for (const auto& keybinding : _keybindings) {
					if (keybinding.group != ControlGroup::kButtonBar || keybinding.modifierKey) {
						continue;
					}

#if defined(ENABLE_SKYRIM_VR)
					if (IsVrControllerDevice(keybinding.inputKey.deviceType)) {
						continue;
					}
#endif

					if ((keybinding.inputKey.deviceType == DeviceType::kGamepad) != isGamepad) {
						continue;
					}

					if (std::ranges::any_of(filtered, [&](const Keybinding& existing) {
							return existing.action == keybinding.action;
						})) {
						continue;
					}

					filtered.push_back(keybinding);
				}
			}
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

		if (REL::Module::IsVR()) {
			// B/Y and Grip on the main-hand wand are the VR equivalents of Flatrim's dedicated Take All
			// and Search keys (Take instead rides the "Activate" key itself via HandleGrab's tap split,
			// and hold-Activate remains the physics grab, so Transfer/Search needed its own button too).
			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kOculusPrimary, VRInput::kBY), .action = QuickLootAction::kTakeAll, .buttonArtOverride = ButtonArtIndex::kOculusB });
			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kVivePrimary, VRInput::kBY), .action = QuickLootAction::kTakeAll, .buttonArtOverride = ButtonArtIndex::kOculusB });
			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kWMRPrimary, VRInput::kBY), .action = QuickLootAction::kTakeAll, .buttonArtOverride = ButtonArtIndex::kOculusB });

			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kOculusPrimary, VRInput::kGrip), .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kVrGrip });
			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kVivePrimary, VRInput::kGrip), .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kVrGrip });
			_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(DeviceType::kWMRPrimary, VRInput::kGrip), .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kVrGrip });

			// Quest/Oculus report grip as kGripAlt (axis 2) rather than kGrip - register both so
			// UpdateMappings can suppress the conflicting vanilla mapping for either code.
			for (auto device : { DeviceType::kOculusPrimary, DeviceType::kVivePrimary, DeviceType::kWMRPrimary }) {
				_keybindings.push_back(Keybinding{ .group = ControlGroup::kButtonBar, .inputKey = DeviceKey::Get(device, VRInput::kGripAlt), .action = QuickLootAction::kTransfer, .buttonArtOverride = ButtonArtIndex::kVrGrip });
			}

			const auto controlMap = RE::ControlMap::GetSingleton();
			for (auto device : { DeviceType::kOculusPrimary, DeviceType::kVivePrimary, DeviceType::kWMRPrimary }) {
				const auto activateKey = controlMap->GetMappedKey("Activate", device);
				if (activateKey == RE::ControlMap::kInvalid) {
					continue;
				}

				const auto activate = DeviceKey::Get(device, activateKey);
				_keybindings.push_back(Keybinding{
					.group = ControlGroup::kButtonBar,
					.inputKey = activate,
					.modifierKey = DeviceKey::Get(device, VRInput::kTrigger),
					.action = QuickLootAction::kUse,
					.buttonArtOverride = ButtonArtIndex::kVrTriggerHold });
				_keybindings.push_back(Keybinding{
					.group = ControlGroup::kButtonBar,
					.inputKey = activate,
					.modifierKey = DeviceKey::Get(device, VRInput::kJoystickTrigger),
					.action = QuickLootAction::kUse,
					.buttonArtOverride = ButtonArtIndex::kVrTriggerHold });
			}
		}

		_keybindings.append_range(Config::UserSettings::GetKeybindings());

		_allInputKeys.clear();
		_allModifierKeys.clear();

		for (size_t i = 0; i < _keybindings.size(); ++i) {
			auto& keybinding = _keybindings[i];

			_allInputKeys.insert(keybinding.inputKey);

			if (keybinding.modifierKey) {
				_allModifierKeys.insert(*keybinding.modifierKey);
			}
		}

		if (REL::Module::IsVR()) {
			// HandleGrab resolves the wand's "Activate" key dynamically every event (its key code
			// isn't fixed, so it can't be registered as a Keybinding up front), but HandleButtonEvent
			// still bails out before calling HandleGrab unless the key is in _allInputKeys - so Take
			// (tap) and the grab hold never actually fired without also adding it here.
			const auto controlMap = RE::ControlMap::GetSingleton();
			for (auto device : { DeviceType::kOculusPrimary, DeviceType::kVivePrimary, DeviceType::kWMRPrimary }) {
				const auto key = controlMap->GetMappedKey("Activate", device);
				if (key != RE::ControlMap::kInvalid) {
					_allInputKeys.insert(DeviceKey::Get(device, key));
				}

				_allInputKeys.insert(DeviceKey::Get(device, VRInput::kTrigger));
				_allInputKeys.insert(DeviceKey::Get(device, VRInput::kJoystickTrigger));
				_allModifierKeys.insert(DeviceKey::Get(device, VRInput::kTrigger));
				_allModifierKeys.insert(DeviceKey::Get(device, VRInput::kJoystickTrigger));
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
#if defined(ENABLE_SKYRIM_VR)
			       KeybindingMatchesEvent(keybinding, deviceType, inputKey) &&
#else
			       keybinding.inputKey.deviceType == deviceType &&
			       keybinding.inputKey.keyCode == inputKey &&
#endif
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

		const auto fakeEvent = RE::ButtonEvent::Create(device, "", idCode, value, heldDownSecs);

		HandleButtonEvent(fakeEvent);

		RE::free(fakeEvent);
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
		if (!event->IsPressed() && _triggerOnActivateRelease) {
			_triggerOnActivateRelease = false;

			// VR has no MCM-configurable keybinding for the wand's Activate button (it's not a
			// fixed key code we can register up front), so tapping it always means Take while
			// the menu is showing rather than relying on a matched Keybinding entry.
			if (REL::Module::IsVR() && LootMenuManager::IsShowing()) {
				LootMenuManager::OnInputAction(IsVrUseModifierPressed() ? QuickLootAction::kUse : QuickLootAction::kTake);
				return true;
			}

			if (keybinding) {
				TriggerKeybinding(keybinding);
			}
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

	bool InputManager::IsVrUseModifierPressed()
	{
#if defined(ENABLE_SKYRIM_VR)
		if (!REL::Module::IsVR()) {
			return false;
		}

		const auto device = GetInputDevice(DeviceType::kOculusPrimary);
		if (!device) {
			return false;
		}

		return device->IsPressed(VRInput::kTrigger) || device->IsPressed(VRInput::kJoystickTrigger);
#else
		return false;
#endif
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
		if (keybinding->action == QuickLootAction::kTransfer && _suppressTransferUntilGripRelease) {
			return;
		}

		if (keybinding->action == QuickLootAction::kTransfer) {
			_suppressTransferUntilGripRelease = true;
		}

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
