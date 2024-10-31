#include "ActivationPrompt.h"

namespace QuickLoot::Behaviors
{
	struct AddMessageHook
	{
		static void thunk(RE::UIMessageQueue* _this, const RE::BSFixedString& menuName, RE::UI_MESSAGE_TYPE type, RE::HUDData* data)
		{
			if (ActivationPrompt::IsBlocked()) {
				data->text = "";
				data->unk40 = false; // enable
			}

			func(_this, menuName, type, data);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	void ActivationPrompt::Install()
	{
		// Copied from https://github.com/powerof3/SimpleActivateSKSE/blob/master/src/Manager.cpp

		REL::Relocation loc{ RELOCATION_ID(39535, 40621), REL::VariantOffset(0x289, 0x280, 0) };
		AddMessageHook::func = SKSE::GetTrampoline().write_call<5>(loc.address(), AddMessageHook::thunk);

		logger::info("Installed {}", typeid(ActivationPrompt).name());
	}
}