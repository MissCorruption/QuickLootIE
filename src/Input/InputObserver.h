#pragma once

namespace QuickLoot::Input
{
	class InputObserver final : public RE::BSTEventSink<RE::InputEvent*>
	{
		static inline bool _isListening = false;

		static InputObserver* GetSingleton()
		{
			static InputObserver instance;
			return &instance;
		}

		InputObserver() = default;
		~InputObserver() override = default;

	public:
		InputObserver(InputObserver&&) = delete;
		InputObserver(const InputObserver&) = delete;
		InputObserver& operator=(InputObserver&&) = delete;
		InputObserver& operator=(const InputObserver&) = delete;

		static void StartListening();
		static void StopListening();

		RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* event, RE::BSTEventSource<RE::InputEvent*>* eventSource) override;
	};
}
