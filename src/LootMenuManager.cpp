#include "LootMenuManager.h"

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/ContainerAnimator.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "Integrations/BetterThirdPersonSelection.h"
#include "LootMenu.h"
#include "MenuVisibilityManager.h"

namespace QuickLoot
{
	bool LootMenuManager::IsShowing()
	{
		return static_cast<bool>(_currentContainer);
	}

	void LootMenuManager::RequestShow(const RE::ObjectRefHandle& container)
	{
		if (!container || container == _currentContainer) {
			return;
		}

		if (API::APIServer::DispatchOpeningLootMenuEvent(container.get().get()) != API::HandleResult::kContinue) {
			logger::info("Opening was canceled by API subscriber");
			RequestHide();
			return;
		}

		Input::InputManager::BlockConflictingInputs();
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		if (!RE::PlayerCamera::GetSingleton()->IsInThirdPerson() ||
			!Integrations::BetterThirdPersonSelection::Is3DWidgetEnabled()) {
			Behaviors::ActivationPrompt::Block();
			logger::info("Blocked conflicting inputs and activation prompt");
		}

		_currentContainer = container;
		const auto index = container == _lastContainer ? _lastSelectedIndex : 0;

		EnsureOpen();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Show(container, index);
		});
	}

	void LootMenuManager::RequestHide()
	{
		Input::InputManager::UnblockConflictingInputs();
		Behaviors::ActivationPrompt::Unblock();
		logger::info("Unblocked input and activation prompt");
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		_currentContainer.reset();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Hide();
		});

		RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}

	void LootMenuManager::RequestRefresh(RefreshFlags flags)
	{
		QueueLootMenuTask([=](LootMenu& menu) {
			menu.QueueRefresh(flags);
		});
	}

	void LootMenuManager::OnInputAction(Input::QuickLootAction action)
	{
		//logger::trace("Input action {}", static_cast<int>(action));

		const auto plugin = SKSE::PluginDeclaration::GetSingleton();

		switch (action) {
		case Input::QuickLootAction::kDisable:
			MenuVisibilityManager::DisableLootMenu(std::string(plugin->GetName()));
			break;

		case Input::QuickLootAction::kEnable:
			MenuVisibilityManager::EnableLootMenu(std::string(plugin->GetName()));
			break;

		default:
			QueueLootMenuTask([=](LootMenu& menu) {
				menu.OnInputAction(action);
			});
			break;
		}
	}

	void LootMenuManager::EnsureOpen()
	{
		if (!RE::UI::GetSingleton()->IsMenuOpen(LootMenu::MENU_NAME)) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
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
