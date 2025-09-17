#pragma once

namespace QuickLoot::Observers
{
	class SneakStateObserver
	{
		static bool SetMoveModeBits(RE::ActorState* state, uint16_t bits);
		static bool ClearMoveModeBits(RE::ActorState* state, uint16_t bits);

		inline static decltype(&SetMoveModeBits) SetMoveModeBits_;
		inline static decltype(&ClearMoveModeBits) ClearMoveModeBits_;

	public:
		SneakStateObserver() = delete;
		~SneakStateObserver() = delete;
		SneakStateObserver(SneakStateObserver&&) = delete;
		SneakStateObserver(const SneakStateObserver&) = delete;
		SneakStateObserver& operator=(SneakStateObserver&&) = delete;
		SneakStateObserver& operator=(const SneakStateObserver&) = delete;

		static void Install();
    };
}
