#include "Input/InputManager.h"

namespace QuickLoot::Input
{
	template <typename TPatch>
	void WritePatch()
	{
		static_assert(std::is_base_of_v<Xbyak::CodeGenerator, TPatch>);

		constexpr uint64_t startOffset = TPatch::patchStart - TPatch::functionStart;
		constexpr uint64_t endOffset = TPatch::patchEnd - TPatch::functionStart;
		constexpr uint64_t size = endOffset - startOffset;
		static_assert(size >= 6);

		const REL::Relocation location{ REL::ID(TPatch::functionId), startOffset };
		REL::safe_fill(location.address(), REL::INT3, size);

		TPatch patch{};

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(location.address(), trampoline.allocate(patch));
	}

	struct PatchSE : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 67253;
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

	void InputManager::Install()
	{
		// We patch a tail call to InputManager::UpdateMappings into
		// ControlMap::RefreshLinkedMappings in order to perform our own post-processing logic.

		if (REL::Module::IsAE()) {
			WritePatch<PatchAE>();
		} else {
			WritePatch<PatchSE>();
		}

		UpdateMappings();

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
	}

	void InputManager::ReloadKeybindings()
	{
		_keybindings.clear();

		// TODO load from settings

		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelUp, 0, QuickLootAction::kScrollUp, false);
		_keybindings.emplace_back(ControlGroup::kMouseWheel, DeviceType::kMouse, MouseButton::kWheelDown, 0, QuickLootAction::kScrollDown, false);

		if constexpr (true) {
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kE, 0, QuickLootAction::kTake, false);
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kR, 0, QuickLootAction::kTakeAll, false);
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kQ, 0, QuickLootAction::kTransfer, false);
		} else {
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kE, 0, QuickLootAction::kTake, false);
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kE, KeyboardKey::kLeftShift, QuickLootAction::kTakeAll, false);
			_keybindings.emplace_back(ControlGroup::kButtonBar, DeviceType::kKeyboard, KeyboardKey::kR, 0, QuickLootAction::kTransfer, false);
		}

		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kUp, 0, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kDown, 0, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kLeft, 0, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kArrowKeys, DeviceType::kKeyboard, KeyboardKey::kRight, 0, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_8, 0, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_2, 0, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_4, 0, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kNumPadArrowKeys, DeviceType::kKeyboard, KeyboardKey::kKP_6, 0, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceType::kKeyboard, KeyboardKey::kPageUp, 0, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kPageKeys, DeviceType::kKeyboard, KeyboardKey::kPageDown, 0, QuickLootAction::kNextPage, false);

		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kUp, 0, QuickLootAction::kScrollUp, true);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kDown, 0, QuickLootAction::kScrollDown, true);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kLeft, 0, QuickLootAction::kPrevPage, false);
		_keybindings.emplace_back(ControlGroup::kDpad, DeviceType::kGamepad, GamepadInput::kRight, 0, QuickLootAction::kNextPage, false);
	}

	Keybinding* InputManager::FindConflictingKeybinding(const UserEventMapping& mapping, DeviceType deviceType)
	{
		const auto it = std::ranges::find_if(_keybindings, [&](const Keybinding& keybinding) {
			return keybinding.deviceType == deviceType && keybinding.inputKey == mapping.inputKey && (keybinding.modifier == mapping.modifier || mapping.modifier == 0);
		});

		return it != _keybindings.end() ? &*it : nullptr;
	}

	void InputManager::WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts)
	{
		const auto controlMap = RE::ControlMap::GetSingleton();
		if (!controlMap) {
			logger::error("Unable to access control map");
			return;
		}

		int contextCount = RE::UserEvents::INPUT_CONTEXT_ID::kTotal;
		if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_1130) == std::strong_ordering::less) {
			contextCount = 17;  // Hardcoded for Skyrim 1.6.640 and lower
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
