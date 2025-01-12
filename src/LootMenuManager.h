#pragma once

#include "Input/Input.h"
#include "LootMenu.h"

namespace QuickLoot
{
	// Interface for interacting with the loot menu outside of UI threads.
	class LootMenuManager
	{
	public:
		LootMenuManager() = delete;
		~LootMenuManager() = delete;
		LootMenuManager(LootMenuManager&&) = delete;
		LootMenuManager(const LootMenuManager&) = delete;
		LootMenuManager& operator=(LootMenuManager&&) = delete;
		LootMenuManager& operator=(const LootMenuManager&) = delete;

		static void Init();

		static bool IsShowing();
		static void RequestShow(const RE::ObjectRefHandle& container);
		static void RequestHide();
		static void RequestRefresh(RefreshFlags flags);

		static void OnInputAction(Input::QuickLootAction action);

		// These should only ever be called from the loot menu itself.
		static void ProcessPendingTasks(LootMenu& menu);
		static void SaveLastSelectedIndex(const RE::ObjectRefHandle& container, int selectedIndex);

	private:
		using LootMenuTask = std::function<void(LootMenu& menu)>;

		static inline std::recursive_mutex _lock{};
		static inline std::vector<LootMenuTask> _taskQueue{};
		static inline RE::ObjectRefHandle _currentContainer{};
		static inline RE::ObjectRefHandle _lastContainer{};
		static inline int _lastSelectedIndex = 0;

		static void EnsureOpen();
		static void QueueLootMenuTask(LootMenuTask task);
	};
}
