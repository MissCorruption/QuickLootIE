#include "ActivationPrompt.h"

#include "RE/U/UI.h"

#if defined(ENABLE_SKYRIM_VR)
#	include "RE/W/WSActivateRollover.h"
#endif

namespace QuickLoot::Behaviors
{
	void ActivationPrompt::HideRolloverIfOpen() noexcept
	{
		// No-op: NiNode / QueueMenuNodeUpdate / Unk_09 soft-hide paths crashed or white-squared.
	}

	void ActivationPrompt::Block() noexcept
	{
		_blocked = true;
	}

	void ActivationPrompt::Unblock() noexcept
	{
		_blocked = false;
	}

	struct AddMessageHook
	{
		static void thunk(RE::UIMessageQueue* _this, const RE::BSFixedString& menuName, RE::UI_MESSAGE_TYPE type, RE::HUDData* data)
		{
			if (ActivationPrompt::IsBlocked()) {
#if defined(ENABLE_SKYRIM_VR)
				if (REL::Module::IsVR() && menuName == RE::WSActivateRollover::MENU_NAME) {
					// Swallow lifecycle messages so the AlwaysOpen WorldSpaceMenu is neither
					// shown/updated nor destroyed via kHide while LootMenu is up.
					switch (type) {
					case RE::UI_MESSAGE_TYPE::kShow:
					case RE::UI_MESSAGE_TYPE::kUpdate:
					case RE::UI_MESSAGE_TYPE::kReshow:
					case RE::UI_MESSAGE_TYPE::kHide:
					case RE::UI_MESSAGE_TYPE::kForceHide:
						return;
					default:
						break;
					}
				}
#endif
				if (data && (data->type == RE::HUD_MESSAGE_TYPE::kSetCrosshairTarget || data->type == RE::HUD_MESSAGE_TYPE::kSetCrosshairTargetTextOnly)) {
					std::string text = data->text.c_str();
					std::ranges::replace(text, '\n', ' ');
					logger::trace("Blocking activation prompt '{}'", text);
					data->text = "";
					data->show = false;
				}
			}

			func(_this, menuName, type, data);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	void ActivationPrompt::Install()
	{
		// We intercept the call to UIMessageQueue::AddMessage in PlayerCharacter::UpdateCrosshairs.
		// This is the same way Simple Activate and Which Key do it.
		// Copied from https://github.com/powerof3/SimpleActivateSKSE/blob/master/src/Manager.cpp

		REL::Relocation loc{ RELOCATION_ID(39535, 40621), REL::VariantOffset(0x289, 0x280, 0x22E) };
		AddMessageHook::func = SKSE::GetTrampoline().write_call<5>(loc.address(), AddMessageHook::thunk);

		logger::info("Installed {}", typeid(ActivationPrompt).name());
	}
}
