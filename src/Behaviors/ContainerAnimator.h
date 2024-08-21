#pragma once

namespace QuickLoot::Behaviors
{
	class ContainerAnimator
	{
	public:
		ContainerAnimator() = delete;
		~ContainerAnimator() = delete;
		ContainerAnimator(ContainerAnimator&&) = delete;
		ContainerAnimator(const ContainerAnimator&) = delete;
		ContainerAnimator& operator=(ContainerAnimator&&) = delete;
		ContainerAnimator& operator=(const ContainerAnimator&) = delete;

		static void OpenContainer(const RE::ObjectRefHandle& container);
		static void CloseContainer(const RE::ObjectRefHandle& container);

	private:
		using OpenState = RE::BGSOpenCloseForm::OPEN_STATE;

		static bool Animate(const RE::ObjectRefHandle& container, OpenState idleState, OpenState waitState, bool open);
		static void WaitAndAnimate(const RE::ObjectRefHandle& container, OpenState idleState, bool open);
	};
}
