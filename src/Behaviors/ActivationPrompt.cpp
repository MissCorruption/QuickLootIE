#include "ActivationPrompt.h"

#include "RE/U/UI.h"

#if defined(ENABLE_SKYRIM_VR)
#	include "RE/W/WSActivateRollover.h"
#endif

namespace QuickLoot::Behaviors
{
#if defined(ENABLE_SKYRIM_VR)
	namespace
	{
		// WorldSpaceMenu::menuNode is at 0x48 on VR runtime objects (IMenu 0x40 + event sink),
		// but SKYRIM_CROSS_VR compiles the member at 0x30. Always relocate when touching native menus.
		RE::NiPointer<RE::NiNode>& RolloverMenuNode(RE::WSActivateRollover* menu) noexcept
		{
			return REL::RelocateMember<RE::NiPointer<RE::NiNode>>(menu, 0x30, 0x48);
		}
	}
#endif

	void ActivationPrompt::SoftHideRollover() noexcept
	{
#if defined(ENABLE_SKYRIM_VR)
		if (!REL::Module::IsVR()) {
			return;
		}

		const auto menu = RE::UI::GetSingleton()->GetMenu<RE::WSActivateRollover>().get();
		if (!menu) {
			return;
		}

		auto& menuNode = RolloverMenuNode(menu);
		if (!menuNode || menuNode->GetAppCulled()) {
			return;
		}

		// Mirrors WorldSpaceMenu::ProcessMessage's kHide NiNode hide bit without posting a UI
		// kHide that destroys the WorldSpaceMenu instance.
		menuNode->SetAppCulled(true);
		menu->Unk_09(RE::UI_MENU_Unk09::kNone);
		_softHidden = true;
#endif
	}

	void ActivationPrompt::SoftShowRollover() noexcept
	{
#if defined(ENABLE_SKYRIM_VR)
		if (!REL::Module::IsVR() || !_softHidden.exchange(false)) {
			return;
		}

		const auto menu = RE::UI::GetSingleton()->GetMenu<RE::WSActivateRollover>().get();
		if (!menu) {
			return;
		}

		auto& menuNode = RolloverMenuNode(menu);
		if (!menuNode) {
			return;
		}

		menuNode->SetAppCulled(false);
#endif
	}

	void ActivationPrompt::HideRolloverIfOpen() noexcept
	{
#if defined(ENABLE_SKYRIM_VR)
		if (!REL::Module::IsVR() || !RE::UI::GetSingleton()->IsMenuOpen(RE::WSActivateRollover::MENU_NAME)) {
			return;
		}

		SoftHideRollover();
#endif
	}

	void ActivationPrompt::Block() noexcept
	{
		_blocked = true;

		if (IsBlocked()) {
			SoftHideRollover();
		}
	}

	void ActivationPrompt::Unblock() noexcept
	{
		_blocked = false;
		SoftShowRollover();
	}

	struct AddMessageHook
	{
		static void thunk(RE::UIMessageQueue* _this, const RE::BSFixedString& menuName, RE::UI_MESSAGE_TYPE type, RE::HUDData* data)
		{
			if (ActivationPrompt::IsBlocked()) {
#if defined(ENABLE_SKYRIM_VR)
				if (REL::Module::IsVR() && menuName == RE::WSActivateRollover::MENU_NAME) {
					// Drop show/update so the prompt cannot reappear, and drop hide/force-hide so
					// the UI framework cannot destroy the WorldSpaceMenu instance while we only
					// want a soft (NiNode) suppress alongside LootMenu on the shared VR graph.
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
