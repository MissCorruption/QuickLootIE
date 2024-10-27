#define DLLEXPORT __declspec(dllexport)

#include <spdlog/sinks/basic_file_sink.h>

#include "Behaviors/ActivationBlocker.h"
#include "Integrations/APIServer.h"
#include "Integrations/Completionist.h"
#include "Integrations/LOTD.h"
#include "LootMenu.h"
#include "LootMenuManager.h"
#include "MenuVisibilityManager.h"
#include "Config/Papyrus.h"

void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kPostLoad:

		// This needs to run before kInputLoaded
		Input::InputManager::Install();

		// Checking the LootMenu as early as possible
		QuickLoot::LootMenuManager::CheckScaleform();

		QuickLoot::API::APIServer::Init(SKSE::GetMessagingInterface());
		break;

	case SKSE::MessagingInterface::kDataLoaded:

		QuickLoot::Config::Papyrus::Init();
		QuickLoot::LootMenu::Register();
		QuickLoot::MenuVisibilityManager::InstallHooks();

		QuickLoot::Behaviors::ActivationBlocker::Install();

		QuickLoot::Integrations::LOTD::Init();
		QuickLoot::Integrations::Completionist::Init();
		break;
	}
}

void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLog();

	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(skse);
	SKSE::AllocTrampoline(1 << 6);

	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}
