#include "ActivationPrompt.h"

#include "RE/U/UI.h"
#include "RE/U/UIMessageQueue.h"

#if defined(ENABLE_SKYRIM_VR)
#	include "RE/W/WSActivateRollover.h"
#endif

namespace QuickLoot::Behaviors
{
	void ActivationPrompt::HideRolloverIfOpen() noexcept
	{
#if defined(ENABLE_SKYRIM_VR)
		if (!REL::Module::IsVR()) {
			return;
		}

		// AlwaysOpen: kHide hides the node; it does not destroy the menu instance.
		if (!RE::UI::GetSingleton()->IsMenuOpen(RE::WSActivateRollover::MENU_NAME)) {
			return;
		}

		RE::UIMessageQueue::GetSingleton()->AddMessage(
			RE::WSActivateRollover::MENU_NAME,
			RE::UI_MESSAGE_TYPE::kHide,
			nullptr);
#endif
	}

	void ActivationPrompt::Block() noexcept
	{
		_blocked = true;

		if (IsBlocked()) {
			HideRolloverIfOpen();
		}
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
				// Swallow show/update so the prompt does not reappear over LootMenu; allow kHide through.
				if (REL::Module::IsVR() && menuName == RE::WSActivateRollover::MENU_NAME) {
					switch (type) {
					case RE::UI_MESSAGE_TYPE::kShow:
					case RE::UI_MESSAGE_TYPE::kUpdate:
					case RE::UI_MESSAGE_TYPE::kReshow:
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
		// Same call site as Simple Activate / Which Key: AddMessage inside UpdateCrosshairs.
		REL::Relocation loc{ RELOCATION_ID(39535, 40621), REL::VariantOffset(0x289, 0x280, 0x22E) };
		AddMessageHook::func = SKSE::GetTrampoline().write_call<5>(loc.address(), AddMessageHook::thunk);

		logger::info("Installed {}", typeid(ActivationPrompt).name());
	}
}
