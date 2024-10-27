#pragma once

#include "Behaviors/ActivationPrompt.h"
#include "Input/InputDisablers.h"
#include "Input/InputListeners.h"

class ViewHandler
{
public:
	ViewHandler() = delete;
	ViewHandler(const ViewHandler&) = default;
	ViewHandler(ViewHandler&&) = default;

	ViewHandler(SKSE::stl::observer<RE::IMenu*> a_menu, RE::ActorHandle a_dst) :
		_menu(a_menu),
		_view(a_menu ? a_menu->uiMovie : nullptr),
		_dst(a_dst)
	{
		assert(_menu != nullptr);
		assert(_view != nullptr);
	}

	ViewHandler& operator=(const ViewHandler&) = default;
	ViewHandler& operator=(ViewHandler&&) = default;

	void SetSource(RE::ObjectRefHandle a_src)
	{
		_src = a_src;
	}

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

	void Enable()
	{
		RE::GPtr safety{ _menu };
		auto task = SKSE::GetTaskInterface();
		task->AddUITask([this, safety]() {
			HideHUD();
			if (!_enabled) {
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
				_disablers.Disable();
				_listeners.Disable();
				_enabled = false;
			}
		});
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
		QuickLoot::Behaviors::ActivationPrompt::Unblock();

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

		QuickLoot::Behaviors::ActivationPrompt::Block();
	}

	SKSE::stl::observer<RE::IMenu*> _menu;
	RE::GPtr<RE::GFxMovieView> _view;
	Input::Disablers _disablers;
	Input::Listeners _listeners;
	RE::ObjectRefHandle _src;
	RE::ActorHandle _dst;
	bool _enabled{ false };
};
