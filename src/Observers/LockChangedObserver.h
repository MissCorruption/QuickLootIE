#pragma once

namespace QuickLoot::Observers
{
	class LockChangedObserver : public RE::BSTEventSink<RE::TESLockChangedEvent>
	{
		static LockChangedObserver* GetSingleton()
		{
			static LockChangedObserver instance;
			return &instance;
		}

		LockChangedObserver() = default;
		~LockChangedObserver() override = default;

	public:
		LockChangedObserver(LockChangedObserver&&) = delete;
		LockChangedObserver(const LockChangedObserver&) = delete;
		LockChangedObserver& operator=(LockChangedObserver&&) = delete;
		LockChangedObserver& operator=(const LockChangedObserver&) = delete;

		static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESLockChangedEvent* a_event, RE::BSTEventSource<RE::TESLockChangedEvent>* a_eventSource) override;
    };
}
