#pragma once

namespace QuickLoot
{
	class LootMenu;

	// Interface for interacting with the loot menu outside of UI threads
	class LootMenuManager
	{
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

		static void RefreshUI()
		{
			auto task = SKSE::GetTaskInterface();
			task->AddTask([] {
				_refreshUI = true;
			});
		}

		static void Close();
		static void Open();

		static void ModSelectedIndex(double a_mod);
		static void ModSelectedPage(double a_mod);

		static void RefreshInventory()
		{
			// Need to delay inventory processing so the game has time to process it before us
			auto task = SKSE::GetTaskInterface();
			task->AddTask([] {
				_refreshInventory = true;
			});
		}

		static void SetContainer(RE::ObjectRefHandle a_container);
		static void TakeStack();
		static void TakeAll();

	protected:
		friend class LootMenu;

		static void Process(LootMenu& a_menu);

	private:
		using Tasklet = std::function<void(LootMenu&)>;

		static void AddTask(Tasklet a_task);

		static [[nodiscard]] RE::GPtr<LootMenu> GetMenu();
		static [[nodiscard]] bool IsOpen();

		static [[nodiscard]] bool ShouldOpen()
		{
			if (!_enabled || IsOpen()) {
				return false;
			}

			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player ||
				player->IsGrabbing() ||
				player->HasActorDoingCommand() ||
				(Settings::CloseInCombat() && player->IsInCombat())) {
				return false;
			}

			return true;
		}

		static inline std::mutex _lock{};
		static inline std::vector<Tasklet> _taskQueue{};
		static inline std::atomic_bool _enabled = true ;
		static inline bool _refreshUI = false;
		static inline bool _refreshInventory = false;
	};
}
