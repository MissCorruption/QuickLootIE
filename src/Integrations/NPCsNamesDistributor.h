#pragma once

namespace QuickLoot::Integrations
{
	class NPCsNamesDistributor
	{
	public:
		NPCsNamesDistributor() = delete;
		~NPCsNamesDistributor() = delete;
		NPCsNamesDistributor(NPCsNamesDistributor const&) = delete;
		NPCsNamesDistributor(NPCsNamesDistributor const&&) = delete;
		NPCsNamesDistributor operator=(NPCsNamesDistributor&) = delete;
		NPCsNamesDistributor operator=(NPCsNamesDistributor&&) = delete;

		static bool Init()
		{
			_api = static_cast<IVNND3*>(RequestPluginAPI(InterfaceVersion::kV3));
			return IsReady();
		}

		static bool IsReady()
		{
			return _api != nullptr;
		}

		static void RevealName(RE::Actor* actor)
		{
			if (_api) {
				//logger::debug("NND: Calling RevealName(0x{:x}, kLooting)", reinterpret_cast<uintptr_t>(actor));
				_api->RevealName(actor, RevealReason::kLooting);
			}
		}

		static std::string GetName(RE::Actor* actor)
		{
			std::string name{};

			if (_api) {
				name = _api->GetName(actor, NameContext::kInventory);
				//logger::debug("NND: GetName(0x{:x}) returned \"{}\"", reinterpret_cast<uintptr_t>(actor), name);
			}
			
			return name.empty() ? actor->GetDisplayFullName() : name;
		}

	private:
		// API implementation adapted from https://github.com/adya/NPCs-Names-Distributor/blob/1d94e317594b40ac3f0f9e152f50e2722d1ea991/include/NND_API.h

		static inline const char* NNDPluginName = "NPCsNamesDistributor";

		// Available NND interface versions
		enum class InterfaceVersion : uint8_t
		{
			kV1,

			/// <summary>
			/// Introduces a new NameContext kDialogueHistory. Attempting to access it in older versions would return name for kOther context instead.
			/// </summary>
			kV2,

			/// <summary>
			/// Introduces RevealReason parameter to RevealName method. NND will ensure that corresponding setting is enabled to allow revealing the name for the provided reason.
			/// </summary>
			kV3
		};

		enum class NameContext : uint8_t
		{
			kCrosshair = 1,
			kCrosshairMinion,

			kSubtitles,
			kDialogue,

			kInventory,

			kBarter,

			kEnemyHUD,

			kOther,

			kDialogueHistory
		};

		/// Reason for revealing the name of an NPC. NND uses it to determine whether the name reveal should be allowed based on mod settings.
		enum class RevealReason : uint8_t
		{
			/// Revealing name while looting a dead NPC.
			kLooting,
			/// Revealing name when entering dialogue with an NPC.
			kDialogue,
			/// Revealing name when pickpocketing an NPC.
			kPickpocket,
			/// Revealing name when an NPC initiates conversation with the player.
			kGreeting
		};

		// NND's modder interface
		class IVNND1
		{
		public:
			/// <summary>
			/// Retrieves a generated name for given actor appropriate in specified context.
			/// Note that NND might not have a name for the actor. In this case an empty string will be returned.
			/// For backward compatibility reasons, V1 version will return actor->GetName() for kEnemyHUD context.
			/// </summary>
			/// <param name="actor">Actor for which the name should be retrieved.</param>
			/// <param name="context">Context in which the name needs to be displayed. Depending on context name might either shortened or formatted differently.</param>
			/// <returns>A name generated for the actor. If actor does not support generated names an empty string will be returned instead.</returns>
			virtual std::string_view GetName(RE::ActorHandle actor, NameContext context) noexcept = 0;

			/// <summary>
			/// Retrieves a generated name for given actor appropriate in specified context.
			/// Note that NND might not have a name for the actor. In this case an empty string will be returned.
			/// For backward compatibility reasons, V1 version will return actor->GetName() for kEnemyHUD context.
			/// </summary>
			/// <param name="actor">Actor for which the name should be retrieved.</param>
			/// <param name="context">Context in which the name needs to be displayed. Depending on context name might either shortened or formatted differently.</param>
			/// <returns>A name generated for the actor. If actor does not support generated names an empty string will be returned instead.</returns>
			virtual std::string_view GetName(RE::Actor* actor, NameContext context) noexcept = 0;

			/// <summary>
			/// Reveals a real name of the given actor to the player. If player already know actor's name this method does nothing.
			/// This method can be used to programatically introduce the actor to the player.
			/// </summary>
			/// <param name="actor">Actor whos name should be revealed.</param>
			virtual void RevealName(RE::ActorHandle actor) noexcept = 0;

			/// <summary>
			/// Reveals a real name of the given actor to the player. If player already know actor's name this method does nothing.
			/// This method can be used to programatically introduce the actor to the player.
			/// </summary>
			/// <param name="actor">Actor whos name should be revealed.</param>
			virtual void RevealName(RE::Actor* actor) noexcept = 0;
		};

		using IVNND2 = IVNND1;

		class IVNND3 : public IVNND2
		{
		public:
			/// <summary>
			/// Reveals a real name of the given actor to the player with a specified reason.
			/// If player already knows actor's name this method does nothing.
			/// The reveal might be blocked based on mod settings and the provided reason.
			/// </summary>
			/// <param name="actor">Actor whose name should be revealed.</param>
			/// <param name="reason">Reason for revealing the name.</param>
			virtual bool RevealName(RE::ActorHandle actor, RevealReason reason) noexcept = 0;

			/// <summary>
			/// Reveals a real name of the given actor to the player with a specified reason.
			/// If player already knows actor's name this method does nothing.
			/// The reveal might be blocked based on mod settings and the provided reason.
			/// </summary>
			/// <param name="actor">Actor whose name should be revealed.</param>
			/// <param name="reason">Reason for revealing the name.</param>
			virtual bool RevealName(RE::Actor* actor, RevealReason reason) noexcept = 0;
		};

		/// <summary>
		/// Request the NND API interface.
		/// Recommended: Send your request during or after SKSEMessagingInterface::kMessage_PostLoad to make sure the dll has already been loaded
		/// </summary>
		/// <param name="a_apiVersion">The interface version to request</param>
		/// <returns>The pointer to the API singleton, or nullptr if request failed</returns>
		[[nodiscard]] static inline void* RequestPluginAPI(const InterfaceVersion a_apiVersion = InterfaceVersion::kV3)
		{
			const auto pluginHandle = GetModuleHandle("NPCsNamesDistributor.dll");
			if (!pluginHandle)
				return nullptr;

			typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);
			const _RequestPluginAPI requestAPIFunction = reinterpret_cast<_RequestPluginAPI>(GetProcAddress(pluginHandle, "RequestPluginAPI"));
			if (!requestAPIFunction)
				return nullptr;

			return requestAPIFunction(a_apiVersion);
		}

		static inline IVNND3* _api;
	};
}
