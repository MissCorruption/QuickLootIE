#pragma once

namespace QuickLoot::Observers
{
	class ContainerObserver : public RE::BSTEventSink<RE::TESContainerChangedEvent>
	{
		static ContainerObserver* GetSingleton()
		{
			static ContainerObserver instance;
			return &instance;
		}

		ContainerObserver() = default;
		~ContainerObserver() override = default;

	public:
		ContainerObserver(ContainerObserver&&) = delete;
		ContainerObserver(const ContainerObserver&) = delete;
		ContainerObserver& operator=(ContainerObserver&&) = delete;
		ContainerObserver& operator=(const ContainerObserver&) = delete;

		static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event, RE::BSTEventSource<RE::TESContainerChangedEvent>* a_eventSource) override;
    };
}
