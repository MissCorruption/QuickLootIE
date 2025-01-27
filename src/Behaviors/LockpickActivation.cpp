#include "LockpickActivation.h"

namespace QuickLoot::Behaviors
{
	struct ActivateRefHook
	{
		static void thunk1(RE::TESObjectREFR* _this, RE::TESObjectREFR* activator, uint8_t a3, RE::TESObjectACTI* a4, int a5, char skipScriptEvent)
		{
			thunk(_this, activator, a3, a4, a5, skipScriptEvent, func1);
		}

		static void thunk2(RE::TESObjectREFR* _this, RE::TESObjectREFR* activator, uint8_t a3, RE::TESObjectACTI* a4, int a5, char skipScriptEvent)
		{
			thunk(_this, activator, a3, a4, a5, skipScriptEvent, func2);
		}

		static inline REL::Relocation<decltype(thunk1)> func1;
		static inline REL::Relocation<decltype(thunk2)> func2;

		__declspec(noinline)
		static void thunk(RE::TESObjectREFR* _this, RE::TESObjectREFR* activator, uint8_t a3, RE::TESObjectACTI* a4, int a5, char skipScriptEvent, const REL::Relocation<decltype(thunk1)>& func)
		{
			if (LockpickActivation::IsBlocked() && _this->GetObjectReference()->Is(RE::FormType::Container)) {
				return;
			}

			func(_this, activator, a3, a4, a5, skipScriptEvent);
		}
	};

	void LockpickActivation::Install()
	{
		// We overwrite two calls to TESObjectREFR::ActivateRef to prevent the container menu from opening.
		// - In TESObjectCONT::Activate -> Called when unlocking with a key.
		// - In LockpickingMenu::Unlock -> Called when succeeding at the lockpicking minigame.
		// Inspired by https://github.com/Umgak/Skyrim-No-Lockpick-Activate/blob/master/src/main.cpp
		// No Lockpick Activate uses ID(17922)+0x2E4 rather than ID(17887)+0x164, but that seems to be the wrong function.

		REL::Relocation loc1{ RELOCATION_ID(17485, 17887), REL::VariantOffset(0x164, 0x164, 0x164) };
		REL::Relocation loc2{ RELOCATION_ID(51088, 51968), REL::VariantOffset(0x1BE, 0x1C0, 0x1BE) };

		// Only install hooks if the calls are still intact. No Lockpick Activate overwrites them with NOP slides,
		// so without this check we'd attempt to call a function with relative offset 0x90909090.

		if (*reinterpret_cast<const uint8_t*>(loc1.address()) == 0xE8) {
			ActivateRefHook::func1 = SKSE::GetTrampoline().write_call<5>(loc1.address(), ActivateRefHook::thunk1);
		}

		if (*reinterpret_cast<const uint8_t*>(loc2.address()) == 0xE8) {
			ActivateRefHook::func2 = SKSE::GetTrampoline().write_call<5>(loc2.address(), ActivateRefHook::thunk2);
		}

		logger::info("Installed {}", typeid(LockpickActivation).name());
	}
}
