#pragma once

namespace QuickLoot::Observers
{
	class CombatStateObserver : public RE::BSTEventSink<RE::TESCombatEvent>
	{
		static CombatStateObserver* GetSingleton()
		{
			static CombatStateObserver instance;
			return &instance;
		}

		CombatStateObserver() = default;
		~CombatStateObserver() override = default;

	public:
		CombatStateObserver(CombatStateObserver&&) = delete;
		CombatStateObserver(const CombatStateObserver&) = delete;
		CombatStateObserver& operator=(CombatStateObserver&&) = delete;
		CombatStateObserver& operator=(const CombatStateObserver&) = delete;

		static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>* a_eventSource) override;
    };
}
