#pragma once

#include <atomic>

namespace QuickLoot::Behaviors
{
	class ActivationBlocker
	{
		static inline std::atomic_bool _blocked = false;

	public:
		ActivationBlocker() = delete;
		~ActivationBlocker() = delete;
		ActivationBlocker(ActivationBlocker&&) = delete;
		ActivationBlocker(const ActivationBlocker&) = delete;
		ActivationBlocker& operator=(ActivationBlocker&&) = delete;
		ActivationBlocker& operator=(const ActivationBlocker&) = delete;

		static void Install();

		static void BlockActivation() noexcept { _blocked = true; }
		static void UnblockActivation() noexcept { _blocked = false; }
		static bool IsActivationBlocked() noexcept { return _blocked; }
	};
}
