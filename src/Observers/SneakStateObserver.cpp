#include "SneakStateObserver.h"

#include "MenuVisibilityManager.h"
#include "REL/Relocation.h"
#include "SKSE/Impl/PCH.h"

namespace QuickLoot::Observers
{
	bool SneakStateObserver::SetMoveModeBits(RE::ActorState* state, uint16_t bits)
	{
		SetMoveModeBits_(state, bits);
		MenuVisibilityManager::OnSneakStateChanged();
		return true;
	}

	bool SneakStateObserver::ClearMoveModeBits(RE::ActorState* state, uint16_t bits) {
		ClearMoveModeBits_(state, bits);
		MenuVisibilityManager::OnSneakStateChanged();
		return true;
	}

	void SneakStateObserver::Install()
	{
		REL::Relocation callsite_SetMoveModeBits(RELOCATION_ID(36926, 37951), REL::VariantOffset(0xE4, 0xA0, 0xE4));
		REL::Relocation callsite_ClearMoveModeBits(RELOCATION_ID(36926, 37951), REL::VariantOffset(0xEB, 0xB2, 0xEB));

		SetMoveModeBits_ = reinterpret_cast<decltype(&SetMoveModeBits)>(callsite_SetMoveModeBits.write_call<5>(SetMoveModeBits));
		ClearMoveModeBits_ = reinterpret_cast<decltype(&ClearMoveModeBits)>(callsite_ClearMoveModeBits.write_call<5>(ClearMoveModeBits));

		logger::info("Installed {}", typeid(SneakStateObserver).name());
	}
}
