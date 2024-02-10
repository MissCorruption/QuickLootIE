#pragma once

#include "Animation/Animation.h"
#include "Input/InputDisablers.h"
#include "Input/InputListeners.h"

class ViewHandler :
	public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
	public Animation::IEventSink
{
public:
	ViewHandler() = delete;
	ViewHandler(const ViewHandler&) = default;
	ViewHandler(ViewHandler&&) = default;

	ViewHandler(stl::observer<RE::IMenu*> a_menu, RE::ActorHandle a_dst) :
		_menu(a_menu),
		_view(a_menu ? a_menu->uiMovie : nullptr),
		_dst(a_dst)
	{
		assert(_menu != nullptr);
		assert(_view != nullptr);

		SetVisible(false);
		Register();
		Evaluate();
	}

	~ViewHandler()
	{
		Unregister();
		ShowHUD();
	}

	ViewHandler& operator=(const ViewHandler&) = default;
	ViewHandler& operator=(ViewHandler&&) = default;

	void SetSource(RE::ObjectRefHandle a_src)
	{
		_src = a_src;
		Evaluate();
	}

protected:
	using EventResult = RE::BSEventNotifyControl;

	EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
	{
		auto intfcStr = RE::InterfaceStrings::GetSingleton();
		if (intfcStr 
			&& a_event 
			&& a_event->menuName == intfcStr->lockpickingMenu 
			&& (!Settings::OpenWhenContainerUnlocked() || a_event->opening)) {
			Close();
		}

		Evaluate();
		return EventResult::kContinue;
	}

	void OnAnimationEvent() override { Evaluate(); }

private:
	enum class Priority : std::size_t
	{
		kDefault,
		kLowest
	};

	enum : std::size_t
	{
		kActivate,
		kName,
		kShowButton,
		kTextOnly,
		kFavorMode,
		kShowCrosshair,
		kWeight,
		kCost,
		kFieldValue,
		kFieldText
	};

	void Register()
	{
		auto menuSrc = RE::UI::GetSingleton();
		if (menuSrc) {
			menuSrc->AddEventSink(this);
		}
	}

	void Unregister()
	{
		auto menuSrc = RE::UI::GetSingleton();
		if (menuSrc) {
			menuSrc->RemoveEventSink(this);
		}
	}

	void Evaluate()
	{
		const auto controlMap = RE::ControlMap::GetSingleton();
		const auto menuControls = RE::MenuControls::GetSingleton();
		const auto src = _src.get();
		const auto dst = _dst.get();
		if (controlMap && menuControls) {
			const auto& priorityStack = controlMap->GetRuntimeData().contextPriorityStack;
			if (!src ||
				src->IsLocked() ||
				src->IsActivationBlocked() ||
				!dst ||
				dst->IsInKillMove() ||
				dst->GetOccupiedFurniture() ||
				menuControls->InBeastForm() ||
				priorityStack.empty() ||
				priorityStack.back() != RE::UserEvents::INPUT_CONTEXT_ID::kGameplay) {
				Disable();
			} else {
				Enable();
			}
		}
	}

	void Enable()
	{
		RE::GPtr safety{ _menu };
		auto task = SKSE::GetTaskInterface();
		task->AddUITask([this, safety]() {
			HideHUD();
			if (!_enabled) {
				AdjustPriority(Priority::kDefault);
				SetVisible(true);
				_disablers.Enable();
				_listeners.Enable();
				_enabled = true;
			}
		});
	}

	void Disable()
	{
		RE::GPtr safety{ _menu };
		auto task = SKSE::GetTaskInterface();
		task->AddUITask([this, safety]() {
			ShowHUD();
			if (_enabled) {
				AdjustPriority(Priority::kLowest);
				SetVisible(false);
				_disablers.Disable();
				_listeners.Disable();
				_enabled = false;
			}
		});
	}

	void SetVisible(bool a_visible)
	{
		if (_view) {
			const auto prev = _view->GetVisible();
			if (prev != a_visible) {
				_view->SetVisible(a_visible);

				if (a_visible) {
					RefreshUI();
				}
			}
		} else {
			assert(false);
		}
	}

	[[nodiscard]] RE::GFxValue GetHUDObject()
	{
		RE::GFxValue object;

		auto ui = RE::UI::GetSingleton();
		auto hud = ui ? ui->GetMenu<RE::HUDMenu>() : nullptr;
		auto view = hud ? hud->uiMovie : nullptr;
		if (view) {
			view->GetVariable(std::addressof(object), "_root.HUDMovieBaseInstance");
		}

		return object;
	}

	void ShowHUD()
	{
		DisableHUDBlocker();

		auto hud = GetHUDObject();
		if (hud.IsObject()) {
			std::array<RE::GFxValue, 10> args;
			args[kActivate] = true;
			args[kShowButton] = true;
			args[kTextOnly] = true;

			const auto src = _src.get();
			const auto objRef = src ? src->GetObjectReference() : nullptr;
			RE::BSString name;
			if (objRef) {
				objRef->GetActivateText(src.get(), name);
			}
			args[kName] = name.empty() ? "" : name.c_str();

			hud.Invoke("SetCrosshairTarget", args);
		}
	}

	void HideHUD()
	{
		auto hud = GetHUDObject();
		if (hud.IsObject()) {
			std::array<RE::GFxValue, 10> args;
			args[kActivate] = false;
			args[kShowButton] = false;
			args[kTextOnly] = true;

			args[kName] = "";

			hud.Invoke("SetCrosshairTarget", args);
		}

		EnableHUDBlocker();
	}

	void AdjustPriority(Priority a_priority);
	void Close();
	void RefreshUI();
	void EnableHUDBlocker();
	void DisableHUDBlocker();

	stl::observer<RE::IMenu*> _menu;
	RE::GPtr<RE::GFxMovieView> _view;
	Input::Disablers _disablers;
	Input::Listeners _listeners;
	RE::ObjectRefHandle _src;
	RE::ActorHandle _dst;
	bool _enabled{ false };
};
