#include "ActivationPrompt.h"

#include "RE/U/UIMessageQueue.h"
#include "RE/W/WSActivateRollover.h"

namespace QuickLoot::Behaviors
{
	void ActivationPrompt::HideWSActivateRollover()
	{
		// For VR we need to hide the prompt manually.
		// The ProcessMessage hook below only ensures that it doesn't open again.
		// It's important that we don't do this immediately, but only once the LootMenu constructor
		// has confirmed that the container is not empty and the loot menu should indeed be displayed.
		if (REL::Module::IsVR() && IsBlocked()) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(
				RE::WSActivateRollover::MENU_NAME,
				RE::UI_MESSAGE_TYPE::kHide,
				nullptr);
		}
	}

	struct AddMessageHook
	{
		static void thunk(RE::UIMessageQueue* _this, const RE::BSFixedString& menuName, RE::UI_MESSAGE_TYPE type, RE::HUDData* data)
		{
			if (ActivationPrompt::IsBlocked() && (data->type == RE::HUD_MESSAGE_TYPE::kSetCrosshairTarget || data->type == RE::HUD_MESSAGE_TYPE::kSetCrosshairTargetTextOnly)) {
				std::string text = data->text.c_str();
				std::ranges::replace(text, '\n', ' ');
				logger::trace("Blocking activation prompt '{}'", text);
				data->text = "";
				data->show = false;
			}

			func(_this, menuName, type, data);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct ProcessMessageHook
	{
		static int thunk(RE::WSActivateRollover* _this, RE::UIMessage* message)
		{
			if (ActivationPrompt::IsBlocked()) {
				// Swallow show/update so the prompt does not reappear over LootMenu; allow kHide through.
				switch (message->type.get()) {
				case RE::UI_MESSAGE_TYPE::kShow:
				case RE::UI_MESSAGE_TYPE::kUpdate:
				case RE::UI_MESSAGE_TYPE::kReshow:
					logger::trace("Blocking activation prompt");
					return 1;
				default:
					break;
				}
			}

			return func(_this, message);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	void ActivationPrompt::Install()
	{
		if (!REL::Module::IsVR()) {
			// We intercept the call to UIMessageQueue::AddMessage in PlayerCharacter::UpdateCrosshairText.
			// This is the same way Simple Activate and Which Key do it.
			// Copied from https://github.com/powerof3/SimpleActivateSKSE/blob/master/src/Manager.cpp

			REL::Relocation loc{ RELOCATION_ID(39535, 40621), REL::VariantOffset(0x289, 0x280, 0x22E) };
			AddMessageHook::func = loc.write_call<5>(AddMessageHook::thunk);
		} else {
			// The above method doesn't seem to work for VR (the hook is never called),
			// so we hook directly into WSActivateRollover::ProcessMessage instead.

			REL::Relocation loc{ RE::VTABLE_WSActivateRollover[0] };
			ProcessMessageHook::func = loc.write_vfunc(0x4, ProcessMessageHook::thunk);
			return;
		}

		logger::info("Installed {}", typeid(ActivationPrompt).name());
	}
}
