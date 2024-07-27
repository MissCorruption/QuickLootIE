#pragma once

namespace QuickLoot::Observers
{
	class MenuObserver : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
		static MenuObserver* GetSingleton()
		{
			static MenuObserver instance;
			return &instance;
		}

		MenuObserver() = default;
		~MenuObserver() override = default;

	public:
		MenuObserver(MenuObserver&&) = delete;
		MenuObserver(const MenuObserver&) = delete;
		MenuObserver& operator=(MenuObserver&&) = delete;
		MenuObserver& operator=(const MenuObserver&) = delete;

		static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* eventSource) override;
    };
}
