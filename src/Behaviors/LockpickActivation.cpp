#include "LockpickActivation.h"

namespace QuickLoot::Behaviors
{
	struct ActivateRefHook
	{
		static void thunk(RE::TESObjectREFR* _this, RE::TESObjectREFR* activator, uint8_t a3, RE::TESObjectACTI* a4, int a5, char skipScriptEvent)
		{
			if (LockpickActivation::IsBlocked()) {
				return;
			}

			// Don't call the original function if the call was overwritten with a bunch of NOPs.
			if (func.address() == 0x90909090) {
				return;
			}

			func(_this, activator, a3, a4, a5, skipScriptEvent);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	void LockpickActivation::Install()
	{
		// We overwrite the call to TESObjectREFR::ActivateRef in LockpickingMenu::Unlock to prevent the container menu from opening.
		// Relocation IDs and offsets taken from https://github.com/Umgak/Skyrim-No-Lockpick-Activate/blob/master/src/main.cpp

		REL::Relocation loc{ RELOCATION_ID(51088, 51968), REL::VariantOffset(0x1BE, 0x1C0, 0) };
		ActivateRefHook::func = SKSE::GetTrampoline().write_call<5>(loc.address(), ActivateRefHook::thunk);

		logger::info("Installed {}", typeid(LockpickActivation).name());
	}
}
