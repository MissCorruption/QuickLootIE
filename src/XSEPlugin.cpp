#define DLLEXPORT __declspec(dllexport)

#include "Animation/Animation.h"
#include "Events/Events.h"
#include "Hooks.h"
#include "Loot.h"
#include "Scaleform/LootMenu.h"
#include "Integrations/LOTD.h"
#include "Integrations/Completionist.h"
#include "Integrations/PluginServer.h"
#include "Papyrus/Papyrus.h"


namespace
{
	class InputHandler :
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		static InputHandler* GetSingleton()
		{
			static InputHandler singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			auto input = RE::BSInputDeviceManager::GetSingleton();
			input->AddEventSink(GetSingleton());
			logger::info("Registered InputHandler"sv);
		}

	protected:
		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override
		{
			using InputType = RE::INPUT_EVENT_TYPE;
			using Keyboard = RE::BSWin32KeyboardDevice::Key;

			if (!a_event) {
				return EventResult::kContinue;
			}

			auto intfcStr = RE::InterfaceStrings::GetSingleton();
			auto ui = RE::UI::GetSingleton();
			if (ui->IsMenuOpen(intfcStr->console)) {
				return EventResult::kContinue;
			}

			for (auto event = *a_event; event; event = event->next) {
				if (event->eventType != InputType::kButton) {
					continue;
				}

				auto button = static_cast<RE::ButtonEvent*>(event);
				if (!button->IsDown() || button->device != RE::INPUT_DEVICE::kKeyboard) {
					continue;
				}

				auto& loot = Loot::GetSingleton();
				switch (button->idCode) {
				case Keyboard::kNum0:
					loot.Enable();
					break;
				case Keyboard::kNum9:
					loot.Disable();
					break;
				default:
					break;
				}
			}

			return EventResult::kContinue;
		}

	private:
		InputHandler() = default;
		InputHandler(const InputHandler&) = delete;
		InputHandler(InputHandler&&) = delete;

		~InputHandler() = default;

		InputHandler& operator=(const InputHandler&) = delete;
		InputHandler& operator=(InputHandler&&) = delete;
	};

	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
#ifndef NDEBUG
			InputHandler::Register();
#endif

			Animation::AnimationManager::Install();

			Events::Register();
			Scaleform::LootMenu::Register();

			QuickLoot::Papyrus::Init();
            QuickLoot::Integrations::LOTD::Init();
			QuickLoot::Integrations::Completionist::Init();
			break;
		}
	}
}

void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(a_level);
	log->flush_on(a_level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());
	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 6);

	auto message = SKSE::GetMessagingInterface();
	if (!message->RegisterListener(MessageHandler)) {
		return false;
	}
	QuickLoot::Integrations::PluginServer::Init(message);
	
	Hooks::Install();
	QuickLoot::Papyrus::Init();

	return true;
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
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}
