#pragma once

#include "Config/SystemSettings.h"

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

		static void Block() noexcept;
		static void Unblock() noexcept;
		static bool IsBlocked() noexcept { return _blocked && Config::SystemSettings::SuppressActivationPrompt(); }

		// Kept for LootMenu::AdvanceMovie; node-level hide proved unsafe on VR.
		static void HideRolloverIfOpen() noexcept;
	};
}
