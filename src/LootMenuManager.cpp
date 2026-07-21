#include "LootMenuManager.h"

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/ContainerAnimator.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "Integrations/BetterThirdPersonSelection.h"
#include "Integrations/NPCsNamesDistributor.h"
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

		if (API::APIServer::DispatchOpeningLootMenuEvent(container) != API::HandleResult::kContinue) {
			logger::info("Opening was canceled by API subscriber");
			RequestHide();
			return;
		}

		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		if (!RE::PlayerCamera::GetSingleton()->IsInThirdPerson() ||
			!Integrations::BetterThirdPersonSelection::Is3DWidgetEnabled()) {
			Behaviors::ActivationPrompt::Block();
		}

		if (const auto actor = skyrim_cast<RE::Actor*>(container.get().get())) {
			Integrations::NPCsNamesDistributor::RevealName(actor);
		}

		_currentContainer = container;
		const auto index = container == _lastContainer ? _lastSelectedIndex : 0;

		// This may trigger another call to RequestShow, so make sure it happens after _currentContainer is set.
		Input::InputManager::BlockConflictingInputs();

		EnsureOpen();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Show(container, index);
		});
	}

	void LootMenuManager::RequestHide()
	{
		if (!IsShowing()) {
			return;
		}

		Behaviors::ActivationPrompt::Unblock();
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		_currentContainer.reset();

		// This may trigger another call to RequestHide, so make sure it happens after _currentContainer is reset.
		Input::InputManager::UnblockConflictingInputs();

		{
			// kHide destroys the instance, so any pending messages would be run when the
			// next loot menu instance is created. Here, we discard them instead.
			std::scoped_lock lock{ _lock };
			_taskQueue.clear();
		}

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
		std::vector<LootMenuTask> tasks;

		{
			// Swap out the queue so queueing new tasks within another task
			// cannot reallocate the vector mid-iteration.
			std::scoped_lock lock{ _lock };
			tasks.swap(_taskQueue);
		}

		for (auto& task : tasks) {
			task(menu);
		}
	}

	void LootMenuManager::QueueLootMenuTask(LootMenuTask task)
	{
		if (!task) {
			return;
		}

		std::scoped_lock lock{ _lock };
		_taskQueue.push_back(std::move(task));
	}
}
