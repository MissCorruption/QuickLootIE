#include "Animation/Animation.h"
#include "Events/Events.h"
#include "Hooks.h"
#include "Input/Input.h"
#include "Loot.h"
#include "Scaleform/Scaleform.h"
#include "LOTD/LOTD.h"
#include "Items/GFxItem.h"

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
			InputHandler::Register();

			Animation::AnimationManager::Install();

			Events::Register();
			Scaleform::Register();

			Settings::LoadSettings();
			LOTD::LoadLists();
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
		{
			Completionist_Integration::RegisterListener();
		}
		break;
		}
	}

#pragma warning(disable: 4702)
	void InitializeLog()
	{
		auto path = logger::log_directory();
		if (!path) {
			stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log", Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
		const auto level = spdlog::level::info;

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
	}
}

/*extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);

	v.UsesAddressLibrary(true);
	//v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}*/

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 6);

	auto message = SKSE::GetMessagingInterface();
	if (!message->RegisterListener(MessageHandler)) {
		return false;
	}

	Hooks::Install();

	return true;
}
