#pragma once

namespace QuickLoot::Observers
{
	class LifeStateObserver
	{
		static LifeStateObserver* GetSingleton()
		{
			static LifeStateObserver instance;
			return &instance;
		}

		LifeStateObserver() = default;
		~LifeStateObserver() = default;

	public:
		LifeStateObserver(LifeStateObserver&&) = delete;
		LifeStateObserver(const LifeStateObserver&) = delete;
		LifeStateObserver& operator=(LifeStateObserver&&) = delete;
		LifeStateObserver& operator=(const LifeStateObserver&) = delete;

		static void Install();
    };
}
