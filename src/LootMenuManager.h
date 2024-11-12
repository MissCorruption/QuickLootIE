#pragma once
#include "Input/Input.h"

namespace QuickLoot
{
	class LootMenu;

	// Interface for interacting with the loot menu outside of UI threads
	class LootMenuManager
	{
		using Tasklet = std::function<void(LootMenu&)>;

		static inline std::mutex _lock{};
		static inline std::vector<Tasklet> _taskQueue{};
		static inline std::atomic_bool _enabled = true;
		static inline bool _refreshUI = false;
		static inline bool _refreshInventory = false;

		static void AddTask(Tasklet a_task);

		static [[nodiscard]] RE::GPtr<LootMenu> GetMenu();
		static [[nodiscard]] bool IsOpen();
		static [[nodiscard]] bool ShouldOpen();

	public:
		LootMenuManager() = delete;
		~LootMenuManager() = delete;
		LootMenuManager(LootMenuManager&&) = delete;
		LootMenuManager(const LootMenuManager&) = delete;
		LootMenuManager& operator=(LootMenuManager&&) = delete;
		LootMenuManager& operator=(const LootMenuManager&) = delete;

		static void Disable()
		{
			_enabled = false;
			Close();
		}

		static void Enable() { _enabled = true; }

		static void CheckScaleform();

		static void RefreshUI();
		static void RefreshInventory();

		static void Open();
		static void Close();

		static void ModSelectedIndex(double a_mod);
		static void ModSelectedPage(double a_mod);

		static void SetContainer(RE::ObjectRefHandle a_container);
		static void TakeStack();
		static void TakeAll();

		static void OnInputAction(Input::QuickLootAction action);

	protected:
		friend class LootMenu;

		static void Process(LootMenu& a_menu);
	};
}
