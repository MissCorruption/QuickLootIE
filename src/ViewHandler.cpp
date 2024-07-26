#include "ViewHandler.h"

#include "Behaviors/ActivationBlocker.h"
#include "LootMenu.h"
#include "LootMenuManager.h"

void ViewHandler::AdjustPriority(Priority a_priority)
{
	auto ui = RE::UI::GetSingleton();
	if (!ui) {
		assert(false);
		return;
	}

	switch (a_priority) {
	case Priority::kDefault:
		_menu->depthPriority =
			QuickLoot::LootMenu::SortPriority();
		break;
	case Priority::kLowest:
		_menu->depthPriority = -1;
		break;
	default:
		assert(false);
		break;
	}

	std::stable_sort(
		ui->menuStack.begin(),
		ui->menuStack.end(),
		[](auto&& a_lhs, auto&& a_rhs) noexcept {
			return a_lhs->depthPriority < a_rhs->depthPriority;
		});
}

void ViewHandler::Close()
{
	QuickLoot::LootMenuManager::Close();
}

void ViewHandler::RefreshUI()
{
	QuickLoot::LootMenuManager::RefreshUI();
}

void ViewHandler::EnableHUDBlocker()
{
	auto& hud = QuickLoot::Behaviors::ActivationBlocker::GetSingleton();
	hud.Enable();
}

void ViewHandler::DisableHUDBlocker()
{
	auto& hud = QuickLoot::Behaviors::ActivationBlocker::GetSingleton();
	hud.Disable();
}
