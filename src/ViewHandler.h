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
			QuickLoot::Behaviors::ActivationPrompt::Block();
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
			QuickLoot::Behaviors::ActivationPrompt::Unblock();
			if (_enabled) {
				_disablers.Disable();
				_listeners.Disable();
				_enabled = false;
			}
		});
	}

	SKSE::stl::observer<RE::IMenu*> _menu;
	RE::GPtr<RE::GFxMovieView> _view;
	Input::Disablers _disablers;
	Input::Listeners _listeners;
	RE::ObjectRefHandle _src;
	RE::ActorHandle _dst;
	bool _enabled{ false };
};
