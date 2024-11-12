#pragma once

#include "Input/Input.h"

namespace Input
{
	class Disablers
	{
	public:
		Disablers() = default;
		Disablers(const Disablers&) = default;
		Disablers(Disablers&&) = default;

		~Disablers() { Disable(); }

		Disablers& operator=(const Disablers&) = default;
		Disablers& operator=(Disablers&&) = default;

		void Enable()
		{
			auto controlMap = RE::ControlMap::GetSingleton();
			if (controlMap) {
				controlMap->ToggleControls(QuickLoot::Input::QUICKLOOT_EVENT_GROUP_FLAG, false);
			}
		}

		void Disable()
		{
			auto controlMap = RE::ControlMap::GetSingleton();
			if (controlMap) {
				controlMap->ToggleControls(QuickLoot::Input::QUICKLOOT_EVENT_GROUP_FLAG, true);
			}
		}
	};
}
