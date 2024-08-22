#include "LootMenuManager.h"

#include "Behaviors/ActivationBlocker.h"
#include "LootMenu.h"

namespace QuickLoot
{
	// TODO: Add version number to scaleform, then check it here
	// If the version is outdated, inform users that an update may be necessary
	void LootMenuManager::CheckScaleform()
	{
		logger::info("Checking Scaleform...");

		if (std::filesystem::exists(LEGACY_SCALEFORM)) {
			logger::error("Old Scaleform found!");

			int result = WIN32::MessageBoxA(nullptr, 
				"An incompatible LootMenu.swf has been found. This is usually due to patches not made for QuickLoot IE. Please uninstall any outdated patches.\n\nExit Game now? (Recommend yes)", 
				"QuickLoot IE Scaleform Check", 
				MB_YESNO);

			if (result == IDYES) {
				logger::info("Exiting game...");
				std::_Exit(EXIT_FAILURE);
				return;
			}
		}
	}


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
				msgQ->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
				Behaviors::ActivationBlocker::UnblockActivation();
			}
		}
	}

	void LootMenuManager::Open()
	{
		if (ShouldOpen()) {
			auto msgQ = RE::UIMessageQueue::GetSingleton();
			if (msgQ) {
				msgQ->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
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
		return ui ? ui->GetMenu<LootMenu>(LootMenu::MENU_NAME) : nullptr;
	}
};
