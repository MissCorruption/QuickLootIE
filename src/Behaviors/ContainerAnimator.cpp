#include "ContainerAnimator.h"

namespace QuickLoot::Behaviors
{
	using enum RE::BGSOpenCloseForm::OPEN_STATE;

	void ContainerAnimator::OpenContainer(const RE::ObjectRefHandle& container)
	{
		if (!Animate(container, kClosed, kClosing, true))
			return;

		const auto containerRefPtr = container.get();
		const auto playerRefPtr = RE::PlayerCharacter::GetSingleton()->GetHandle().get();

		containerRefPtr->InitChildActivates(playerRefPtr.get());

		const auto events = RE::ScriptEventSourceHolder::GetSingleton();
		events->SendActivateEvent(containerRefPtr, playerRefPtr);
		events->SendOpenCloseEvent(containerRefPtr, playerRefPtr, true);

		RE::ChestsLooted::SendEvent();
	}

	void ContainerAnimator::CloseContainer(const RE::ObjectRefHandle& container)
	{
		if (!Animate(container, kOpen, kOpening, false))
			return;

		const auto containerRefPtr = container.get();
		const auto playerRefPtr = RE::PlayerCharacter::GetSingleton()->GetHandle().get();

		const auto events = RE::ScriptEventSourceHolder::GetSingleton();
		events->SendOpenCloseEvent(containerRefPtr, playerRefPtr, true);
	}

	bool ContainerAnimator::Animate(const RE::ObjectRefHandle& container, OpenState idleState, OpenState waitState, bool open)
	{
		const auto containerRef = container.get().get();
		if (!containerRef)
			return false;

		const auto state = RE::BGSOpenCloseForm::GetOpenState(containerRef);

		if (state == idleState) {
			RE::BGSOpenCloseForm::SetOpenState(containerRef, open, false);
			return true;
		}

		if (state == waitState) {
			std::thread thread(WaitAndAnimate, container, idleState, open);
			thread.detach();
			return true;
		}

		return false;
	}

	void ContainerAnimator::WaitAndAnimate(const RE::ObjectRefHandle& container, OpenState idleState, bool open)
	{
		const auto containerRef = container.get().get();
		if (!containerRef)
			return;

		const auto start = std::chrono::system_clock::now();
		constexpr auto timeout = std::chrono::milliseconds(1000);

		while (std::chrono::system_clock::now() < start + timeout) {
			if (RE::BGSOpenCloseForm::GetOpenState(containerRef) != idleState) {
				// Wait until the idle state has been reached to start the animation
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			SKSE::GetTaskInterface()->AddTask([=, &container] {
				const auto sequence = containerRef->GetSequence(open ? "Open" : "Close");
				if (sequence && sequence->Animating())
					return;

				RE::BGSOpenCloseForm::SetOpenState(containerRef, open, false);
			});

			break;
		}
	}
}
