#pragma once

namespace QuickLoot::Observers
{
	class LifeStateObserver
	{
	public:
		LifeStateObserver() = delete;
		~LifeStateObserver() = delete;
		LifeStateObserver(LifeStateObserver&&) = delete;
		LifeStateObserver(const LifeStateObserver&) = delete;
		LifeStateObserver& operator=(LifeStateObserver&&) = delete;
		LifeStateObserver& operator=(const LifeStateObserver&) = delete;

		static void Install();
    };
}
