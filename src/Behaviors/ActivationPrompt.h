#pragma once

#include <atomic>

namespace QuickLoot::Behaviors
{
	class ActivationPrompt
	{
		static inline std::atomic_bool _blocked = false;

	public:
		ActivationPrompt() = delete;
		~ActivationPrompt() = delete;
		ActivationPrompt(ActivationPrompt&&) = delete;
		ActivationPrompt(const ActivationPrompt&) = delete;
		ActivationPrompt& operator=(ActivationPrompt&&) = delete;
		ActivationPrompt& operator=(const ActivationPrompt&) = delete;

		static void Install();

		static void Block() noexcept { _blocked = true; }
		static void Unblock() noexcept { _blocked = false; }
		static bool IsBlocked() noexcept { return _blocked; }
	};
}
