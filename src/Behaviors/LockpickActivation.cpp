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

		static void thunk(RE::TESObjectREFR* _this, RE::TESObjectREFR* activator, uint8_t a3, RE::TESObjectACTI* a4, int a5, char skipScriptEvent, const REL::Relocation<decltype(thunk1)>& func)
		{
			if (LockpickActivation::IsBlocked() && _this->GetObjectReference()->Is(RE::FormType::Container)) {
				return;
			}

			// Don't call the original function if the call was overwritten with a bunch of NOPs.
			if (func.address() == 0x90909090) {
				return;
			}

			func(_this, activator, a3, a4, a5, skipScriptEvent);
		}
	};

	void LockpickActivation::Install()
	{
		// We overwrite the calls to TESObjectREFR::ActivateRef in LockpickingMenu::Unlock and TESObjectCONT::Activate to prevent the container menu from opening.
		// Relocation IDs and offsets taken from https://github.com/Umgak/Skyrim-No-Lockpick-Activate/blob/master/src/main.cpp

		// SNLA uses ID(17922)+0x2E4 rather than ID(17887)+0x164, but that seems to be the wrong function.
		REL::Relocation loc1{ RELOCATION_ID(17485, 17887), REL::VariantOffset(0x164, 0x164, 0) };
		REL::Relocation loc2{ RELOCATION_ID(51088, 51968), REL::VariantOffset(0x1BE, 0x1C0, 0) };

		ActivateRefHook::func1 = SKSE::GetTrampoline().write_call<5>(loc1.address(), ActivateRefHook::thunk1);
		ActivateRefHook::func2 = SKSE::GetTrampoline().write_call<5>(loc2.address(), ActivateRefHook::thunk2);

		logger::info("Installed {}", typeid(LockpickActivation).name());
	}
}
