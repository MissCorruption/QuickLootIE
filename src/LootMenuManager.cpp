#include "LootMenuManager.h"

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/ContainerAnimator.h"
#include "Input/InputManager.h"
#include "Input/InputObserver.h"
#include "Integrations/APIServer.h"
#include "LootMenu.h"

namespace QuickLoot
{
	bool LootMenuManager::IsOpen()
	{
		return static_cast<bool>(GetMenu());
	}

	void LootMenuManager::RequestOpen(const RE::ObjectRefHandle& container)
	{
		if (!container || container == _currentContainer) {
			return;
		}

		if (API::APIServer::DispatchOpeningLootMenuEvent(container.get()) != HandleResult::kContinue) {
			logger::info("Opening was cancelled by API subscriber");
			RequestClose();
			return;
		}

		if (!IsOpen()) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);

			Input::InputManager::BlockConflictingInputs();
			Input::InputObserver::StartListening();
			Behaviors::ActivationPrompt::Block();
		}

		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);
		_currentContainer = container;
		const auto index = container == _lastContainer ? _lastSelectedIndex : 0;

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.SetContainer(container, index);
		});
	}

	void LootMenuManager::RequestClose()
	{
		if (IsOpen()) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);

			Behaviors::ActivationPrompt::Unblock();
			Input::InputManager::UnblockConflictingInputs();
			Input::InputObserver::StopListening();
		}

		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);
		_currentContainer.reset();
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

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.OnInputAction(action);
		});
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

	RE::GPtr<LootMenu> LootMenuManager::GetMenu()
	{
		return RE::UI::GetSingleton()->GetMenu<LootMenu>(LootMenu::MENU_NAME);
	}
};
