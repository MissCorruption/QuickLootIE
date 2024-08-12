#include "LootMenuManager.h"

#include "Behaviors/ActivationBlocker.h"
#include "LootMenu.h"

namespace QuickLoot
{
	void LootMenuManager::RefreshUI()
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([] {
			_refreshUI = true;
		});
	}

	void LootMenuManager::Close()
	{
		if (IsOpen()) {
			auto msgQ = RE::UIMessageQueue::GetSingleton();
			if (msgQ) {
				msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kHide, nullptr);
				Behaviors::ActivationBlocker::UnblockActivation();
			}
		}
	}

	void LootMenuManager::Open()
	{
		if (ShouldOpen()) {
			auto msgQ = RE::UIMessageQueue::GetSingleton();
			if (msgQ) {
				msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kShow, nullptr);
				Behaviors::ActivationBlocker::BlockActivation();
			}
		}
	}

	void LootMenuManager::ModSelectedIndex(double a_mod)
	{
		AddTask([a_mod](LootMenu& a_menu) {
			a_menu.ModSelectedIndex(a_mod);
		});
	}

	void LootMenuManager::ModSelectedPage(double a_mod)
	{
		AddTask([a_mod](LootMenu& a_menu) {
			a_menu.ModSelectedPage(a_mod);
		});
	}

	void LootMenuManager::RefreshInventory()
	{
		// Need to delay inventory processing so the game has time to process it before us
		auto task = SKSE::GetTaskInterface();
		task->AddTask([] {
			_refreshInventory = true;
		});
	}

	void LootMenuManager::SetContainer(RE::ObjectRefHandle a_container)
	{
		AddTask([a_container](LootMenu& a_menu) {
			a_menu.SetContainer(a_container);
		});
	}

	void LootMenuManager::TakeStack()
	{
		AddTask([](LootMenu& a_menu) {
			a_menu.TakeStack();
		});
	}

	void LootMenuManager::TakeAll()
	{
		AddTask([](LootMenu& a_menu) {
			a_menu.TakeAll();
		});
	}

	void LootMenuManager::Process(LootMenu& a_menu)
	{
		if (!_taskQueue.empty()) {
			for (auto& task : _taskQueue) {
				task(a_menu);
			}
			_taskQueue.clear();
		}

		if (_refreshUI) {
			a_menu.RefreshUI();
		} else if (_refreshInventory) {
			a_menu.RefreshInventory();
		}

		_refreshUI = false;
		_refreshInventory = false;
	}

	void LootMenuManager::AddTask(Tasklet a_task)
	{
		Open();
		std::scoped_lock l{ _lock };
		_taskQueue.push_back(std::move(a_task));
	}

	bool LootMenuManager::IsOpen()
	{
		return static_cast<bool>(GetMenu());
	}

	bool LootMenuManager::ShouldOpen()
	{
		return _enabled && !IsOpen();
	}

	RE::GPtr<LootMenu> LootMenuManager::GetMenu()
	{
		auto ui = RE::UI::GetSingleton();
		return ui ? ui->GetMenu<LootMenu>(LootMenu::MenuName()) : nullptr;
	}
};
