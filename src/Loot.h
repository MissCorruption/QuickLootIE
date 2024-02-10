#pragma once

namespace Scaleform
{
	class LootMenu;
}

// Interface for interacting with the loot menu outside of UI threads
class Loot
{
private:
	using LootMenu = Scaleform::LootMenu;

public:
	static Loot& GetSingleton()
	{
		static Loot singleton;
		return singleton;
	}

	void Disable()
	{
		_enabled = false;
		Close();
	}

	void Enable() { _enabled = true; }

	void RefreshUI()
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([this]() {
			_refreshUI = true;

		});
	}

	void Close();
	void Open();

	void ModSelectedIndex(double a_mod);
	void ModSelectedPage(double a_mod);

	void RefreshInventory()
	{
		// Need to delay inventory processing so the game has time to process it before us
		auto task = SKSE::GetTaskInterface();
		task->AddTask([this]() {
			_refreshInventory = true;
		});
	}

	void SetContainer(RE::ObjectRefHandle a_container);
	void TakeStack();

protected:
	friend class LootMenu;

	void Process(LootMenu& a_menu);

private:
	using Tasklet = std::function<void(LootMenu&)>;

	Loot() = default;
	Loot(const Loot&) = delete;
	Loot(Loot&&) = delete;

	~Loot() = default;

	Loot& operator=(const Loot&) = delete;
	Loot& operator=(Loot&&) = delete;

	void AddTask(Tasklet a_task);

	[[nodiscard]] RE::GPtr<LootMenu> GetMenu() const;
	[[nodiscard]] bool IsOpen() const;

	[[nodiscard]] bool ShouldOpen() const
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

	mutable std::mutex _lock;
	std::vector<Tasklet> _taskQueue;
	std::atomic_bool _enabled{ true };
	bool _refreshUI{ false };
	bool _refreshInventory{ false };
};
