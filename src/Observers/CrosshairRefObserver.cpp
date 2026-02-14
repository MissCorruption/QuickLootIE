#include "CrosshairRefObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	using enum RE::BSEventNotifyControl;

	RE::ObjectRefHandle GetCurrentVRTarget()
	{
		const auto rightHanded = RE::PlayerCharacter::GetSingleton()->GetVRPlayerRuntimeData().isRightHandMainHand;
		const auto hand = rightHanded ? RE::VR_DEVICE::kRightController : RE::VR_DEVICE::kLeftController;
		const auto crosshair = RE::CrosshairPickData::GetSingleton();

		auto currentTargetRef = crosshair->grabPickRef[hand];

		if (!currentTargetRef) {
			currentTargetRef = crosshair->targetActor[hand];
		}

		if (!currentTargetRef) {
			currentTargetRef = crosshair->target[hand];
		}

		return currentTargetRef;
	}

	void CrosshairRefObserver::StartVRTargetPollThread()
	{
		_vrPollRunning = true;

		std::thread t([] {
			bool isReady = true;

			while (_vrPollRunning) {
				if (isReady) {
					isReady = false;

					SKSE::GetTaskInterface()->AddTask([&] {
						isReady = true;
						MenuVisibilityManager::OnCrosshairRefChanged(GetCurrentVRTarget());
					});
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(15));
			}
		});

		t.detach();
	}

	void CrosshairRefObserver::StopVRTargetPollThread()
	{
		_vrPollRunning = false;
	}

	void CrosshairRefObserver::Install()
	{
		if (REL::Module::IsVR()) {
			StartVRTargetPollThread();
		} else {
			SKSE::GetCrosshairRefEventSource()->AddEventSink(GetSingleton());
		}

		logger::info("Installed");
	}

	RE::BSEventNotifyControl CrosshairRefObserver::ProcessEvent(
		const SKSE::CrosshairRefEvent* event,
		RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
		auto handle = RE::ObjectRefHandle();

		if (event && event->crosshairRef) {
			handle = event->crosshairRef->CreateRefHandle();
		}

		MenuVisibilityManager::OnCrosshairRefChanged(handle);

		return kContinue;
	}
}
