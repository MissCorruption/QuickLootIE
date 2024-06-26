#include "Scaleform/LootMenu.h"

#include "Loot.h"

namespace Scaleform
{
	void LootMenu::Close()
	{
		auto& loot = Loot::GetSingleton();
		loot.Close();
	}

	void LootMenu::ProcessDelegate()
	{
		auto& loot = Loot::GetSingleton();
		loot.Process(*this);
	}

	void LootMenu::QueueInventoryRefresh()
	{
		auto& loot = Loot::GetSingleton();
		loot.RefreshInventory();
	}

	void LootMenu::QueueUIRefresh()
	{
		auto& loot = Loot::GetSingleton();
		loot.RefreshUI();
	}

	class ProxyFunctionHandler : public RE::GFxFunctionHandler
	{
		std::function<void(Params& params)> _handler;

		void Call(Params& params) override
		{
			_handler(params);
		}

	public:
		ProxyFunctionHandler(const std::function<void(Params& params)>& handler) :
			_handler(handler) {}
	};

	void LootMenu::UtilsLog(const RE::GFxFunctionHandler::Params& params)
	{
		if (params.argCount == 0) {
			return;
		}

		if (!params.args[0].IsString()) {
			return;
		}

		logger::info("! {}", params.args[0].GetString());
	}

	void LootMenu::InjectUtilsClass()
	{
		static ProxyFunctionHandler logger([this](auto& params) { UtilsLog(params); });

		RE::GFxValue quickLootUtilsClass;
		RE::GFxValue quickLootLogFunction;

		_view->CreateObject(&quickLootUtilsClass);
		_view->CreateFunction(&quickLootLogFunction, &logger, nullptr);

		_view->SetVariable("_global.QuickLoot.Utils", quickLootUtilsClass);
		_view->SetVariable("_global.QuickLoot.Utils.log", quickLootLogFunction);
	}
}
