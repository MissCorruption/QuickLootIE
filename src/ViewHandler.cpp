#include "ViewHandler.h"

#include "HUDManager.h"
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
			Scaleform::LootMenu::SortPriority();
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
	auto& loot = LootMenuManager::GetSingleton();
	loot.Close();
}

void ViewHandler::RefreshUI()
{
	auto& loot = LootMenuManager::GetSingleton();
	loot.RefreshUI();
}

void ViewHandler::EnableHUDBlocker()
{
	auto& hud = HUDManager::GetSingleton();
	hud.Enable();
}

void ViewHandler::DisableHUDBlocker()
{
	auto& hud = HUDManager::GetSingleton();
	hud.Disable();
}
