#include "LootMenuManager.h"

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/ContainerAnimator.h"
#include "Config/SystemSettings.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "LootMenu.h"
#include "MenuVisibilityManager.h"

namespace QuickLoot
{
	void LootMenuManager::Init()
	{
		if (Config::SystemSettings::EnableMenuCaching()) {
			logger::info("Preloading menu");
			EnsureOpen();
		}
	}

	bool LootMenuManager::IsOpen()
	{
		return static_cast<bool>(_currentContainer);
	}

	void LootMenuManager::EnsureOpen()
	{
		if (!RE::UI::GetSingleton()->IsMenuOpen(LootMenu::MENU_NAME)) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
		}
	}

	void LootMenuManager::RequestOpen(const RE::ObjectRefHandle& container)
	{
		if (!container || container == _currentContainer) {
			return;
		}

		if (API::APIServer::DispatchOpeningLootMenuEvent(container.get()) != HandleResult::kContinue) {
			logger::info("Opening was canceled by API subscriber");
			RequestClose();
			return;
		}

		Input::InputManager::BlockConflictingInputs();
		Behaviors::ActivationPrompt::Block();
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		_currentContainer = container;
		const auto index = container == _lastContainer ? _lastSelectedIndex : 0;

		EnsureOpen();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Show(container, index);
		});
	}

	void LootMenuManager::RequestClose()
	{
		Input::InputManager::UnblockConflictingInputs();
		Behaviors::ActivationPrompt::Unblock();
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		_currentContainer.reset();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Hide();
		});

		if (!Config::SystemSettings::EnableMenuCaching()) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}

	void LootMenuManager::RequestRefresh(RefreshFlags flags)
	{
		QueueLootMenuTask([=](LootMenu& menu) {
			menu.QueueRefresh(flags);
		});
	}

	void LootMenuManager::OnInputAction(Input::QuickLootAction action)
	{
		logger::trace("Input action {}", static_cast<int>(action));

		switch (action) {
		case Input::QuickLootAction::kDisable:
			MenuVisibilityManager::DisableLootMenu(std::string(Plugin::NAME));
			break;

		case Input::QuickLootAction::kEnable:
			MenuVisibilityManager::EnableLootMenu(std::string(Plugin::NAME));
			break;

		default:
			QueueLootMenuTask([=](LootMenu& menu) {
				menu.OnInputAction(action);
			});
			break;
		}
	}

	void LootMenuManager::SaveLastSelectedIndex(const RE::ObjectRefHandle& container, int selectedIndex)
	{
		if (container.get()) {
			_lastContainer = container;
			_lastSelectedIndex = selectedIndex;
		}
	}

	void LootMenuManager::ProcessPendingTasks(LootMenu& menu)
	{
		std::scoped_lock lock{ _lock };

		if (_taskQueue.empty()) {
			return;
		}

		for (auto& task : _taskQueue) {
			task(menu);
		}

		_taskQueue.clear();
	}

	void LootMenuManager::QueueLootMenuTask(LootMenuTask task)
	{
		std::scoped_lock lock{ _lock };

		_taskQueue.push_back(std::move(task));
	}
}
