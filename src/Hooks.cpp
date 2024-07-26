#include "Hooks.h"

#include "Hooks/ActivationBlocker.h"
#include "Input/Input.h"

namespace Hooks
{
	void Install()
	{
		QuickLoot::Hooks::ActivationBlocker::Install();
		Input::InputManager::Install();
		logger::info("Installed all hooks"sv);
	}
}
