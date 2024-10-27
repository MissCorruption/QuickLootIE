#include "LootMenu.h"

#include "LootMenuManager.h"

namespace QuickLoot
{
	void LootMenu::Close()
	{
        LootMenuManager::Close();
	}

	void LootMenu::ProcessDelegate()
	{
        LootMenuManager::Process(*this);
	}

	void LootMenu::QueueInventoryRefresh()
	{
        LootMenuManager::RefreshInventory();
	}

	void LootMenu::QueueUIRefresh()
	{
        LootMenuManager::RefreshUI();
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
