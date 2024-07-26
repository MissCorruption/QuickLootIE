#include "LootMenuManager.h"

#include "Behaviors/ActivationBlocker.h"
#include "LootMenu.h"

namespace QuickLoot
{
	void LootMenuManager::Close()
	{
		if (IsOpen()) {
			auto msgQ = RE::UIMessageQueue::GetSingleton();
			if (msgQ) {
				msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kHide, nullptr);
				auto& hud = Behaviors::ActivationBlocker::GetSingleton();
				hud.Disable();
			}
		}
	}

	void LootMenuManager::Open()
	{
		if (ShouldOpen()) {
			auto msgQ = RE::UIMessageQueue::GetSingleton();
			if (msgQ) {
				msgQ->AddMessage(LootMenu::MenuName(), RE::UI_MESSAGE_TYPE::kShow, nullptr);
				auto& hud = Behaviors::ActivationBlocker::GetSingleton();
				hud.Enable();
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

	auto LootMenuManager::GetMenu()
		-> RE::GPtr<LootMenu>
	{
		auto ui = RE::UI::GetSingleton();
		return ui ? ui->GetMenu<LootMenu>(LootMenu::MenuName()) : nullptr;
	}
};
