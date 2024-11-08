#pragma once

#include <atomic>

namespace QuickLoot::Behaviors
{
	class LockpickActivation
	{
		static inline std::atomic_bool _blocked = true;

	public:
		LockpickActivation() = delete;
		~LockpickActivation() = delete;
		LockpickActivation(LockpickActivation&&) = delete;
		LockpickActivation(const LockpickActivation&) = delete;
		LockpickActivation& operator=(LockpickActivation&&) = delete;
		LockpickActivation& operator=(const LockpickActivation&) = delete;

		static void Install();

		static void Block() noexcept { _blocked = true; }
		static void Unblock() noexcept { _blocked = false; }
		static bool IsBlocked() noexcept { return _blocked; }
	};
}
