#pragma once

/*
	Header File for QuickLoot IE integration
*/

namespace QuickLoot::API
{
	struct ItemStack
	{
		// This is a pointer to the inventory entry
		RE::InventoryEntryData* entry;
		// This is set if the inventory entry is for an item the NPC dropped on the floor.
		RE::ObjectRefHandle dropRef;
	};

	enum class ActionLabelKind : uint8_t
	{
		kUse = 0,
		kTake = 1,
		kTakeAll = 2,
		kTransfer = 3,
	};

	enum class OverrideState : int8_t
	{
		kNoChange = -1,
		kFalse = 0,
		kTrue = 1,
	};

	namespace Events
	{
		enum class HandleResult : uint8_t
		{
			kContinue = 0,
			kStop = 1
		};

		struct InputActionEvent
		{
			RE::ObjectRefHandle container;
			ActionLabelKind action;
			// Set this to HandleResult::kStop to suppress the default action behavior.
			HandleResult result = HandleResult::kContinue;
		};

		struct TakingItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
			// Set this to HandleResult::kStop to prevent the item from being taken.
			HandleResult result = HandleResult::kContinue;
		};

		struct TakeItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
		};

		struct SelectItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
		};

		struct OpeningLootMenuEvent
		{
			RE::ObjectRefHandle container;
			// Set this to HandleResult::kStop to prevent the loot menu from opening.
			HandleResult result = HandleResult::kContinue;
		};

		struct OpenLootMenuEvent
		{
			RE::ObjectRefHandle container;
		};

		struct CloseLootMenuEvent
		{
			RE::ObjectRefHandle container;
		};

		struct InvalidateLootMenuEvent
		{
			RE::ObjectRefHandle container;
			const RE::BSTArray<ItemStack>& inventory;
		};

		struct ModifyInventoryEvent
		{
			RE::ObjectRefHandle container;
			// Modify this array to change what is displayed in the item list.
			// Each ItemStack owns its InventoryEntryData object.
			// - If you remove entries, make sure to delete their InventoryEntryData objects to avoid leaking memory.
			// - If you add entries, allocate the InventoryEntryData objects with the new operator. QuickLoot will delete them once they are no longer needed.
			RE::BSTArray<ItemStack>& inventory;
		};

		struct PopulateInfoBarEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if the container is empty.
			const ItemStack* stack;
			// Populate this array with text you want to display in the info bar. Some HTML is supported.
			RE::BSTArray<RE::BSString> result;
		};

		struct ButtonDefinition
		{
			RE::BSString label;
			// For a list of valid values, see https://github.com/MissCorruption/QuickLootIE/blob/main/src/Input/ButtonArtIndex.h
			uint16_t buttonArtIndex;
		};

		struct PopulateButtonBarEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if the container is empty.
			const ItemStack* stack;
			// Populate this array with buttons you want to add.
			RE::BSTArray<ButtonDefinition> result;
		};

		struct ResolveStealingStateEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if there is no selection.
			const ItemStack* stack;
			// Set to kFalse or kTrue to override menu stealing context.
			OverrideState stealingContext = OverrideState::kNoChange;
			// Set to kFalse or kTrue to override item stolen marker.
			OverrideState itemStolen = OverrideState::kNoChange;
		};

		struct ResolveActionLabelEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if there is no selection.
			const ItemStack* stack;
			ActionLabelKind action;
			bool stealingContext;
			// Set to a non-empty string to override the action label.
			RE::BSString label;
		};

		template <typename TEvent>
		using EventHandler = void (*)(TEvent* e);

		using TakingItemHandler = EventHandler<TakingItemEvent>;
		using TakeItemHandler = EventHandler<TakeItemEvent>;
		using SelectItemHandler = EventHandler<SelectItemEvent>;
		using OpeningLootMenuHandler = EventHandler<OpeningLootMenuEvent>;
		using OpenLootMenuHandler = EventHandler<OpenLootMenuEvent>;
		using CloseLootMenuHandler = EventHandler<CloseLootMenuEvent>;
		using InvalidateLootMenuHandler = EventHandler<InvalidateLootMenuEvent>;
		using ModifyInventoryHandler = EventHandler<ModifyInventoryEvent>;
		using PopulateInfoBarHandler = EventHandler<PopulateInfoBarEvent>;
		using PopulateButtonBarHandler = EventHandler<PopulateButtonBarEvent>;
		using InputActionHandler = EventHandler<InputActionEvent>;
		using ResolveStealingStateHandler = EventHandler<ResolveStealingStateEvent>;
		using ResolveActionLabelHandler = EventHandler<ResolveActionLabelEvent>;
	}

	using namespace Events;

	class QuickLootAPI
	{
	public:
		QuickLootAPI() = delete;
		~QuickLootAPI() = delete;
		QuickLootAPI(QuickLootAPI const&) = delete;
		QuickLootAPI(QuickLootAPI const&&) = delete;
		QuickLootAPI operator=(QuickLootAPI&) = delete;
		QuickLootAPI operator=(QuickLootAPI&&) = delete;

		static constexpr const char* SERVER_PLUGIN_NAME = "QuickLootIE";

		// Call this before any other API function and pass your own plugin name.
		static bool Init(const char* plugin)
		{
			using GetInterfaceProcV20 = InterfaceV20* (*)();
			using GetInterfaceProcV21 = InterfaceV21* (*)();

			const auto dllHandle = GetModuleHandleA(SERVER_PLUGIN_NAME);
			const auto getInterfaceProcV21 = reinterpret_cast<GetInterfaceProcV21>(GetProcAddress(dllHandle, "GetQuickLootInterfaceV21"));
			const auto getInterfaceProcV20 = reinterpret_cast<GetInterfaceProcV20>(GetProcAddress(dllHandle, "GetQuickLootInterfaceV20"));

			if (getInterfaceProcV21) {
				_plugin = plugin;
				_interfaceV21 = getInterfaceProcV21();
				_interface = _interfaceV21;
			} else if (getInterfaceProcV20) {
				_plugin = plugin;
				_interfaceV21 = nullptr;
				_interface = getInterfaceProcV20();
			}

			return IsReady();
		}

		static bool IsReady()
		{
			return _interface;
		}

		static void DisableLootMenu()
		{
			if (_interface) {
				_interface->DisableLootMenu(_plugin);
			}
		}

		static void EnableLootMenu()
		{
			if (_interface) {
				_interface->EnableLootMenu(_plugin);
			}
		}

		static void RegisterTakingItemHandler(TakingItemHandler handler)
		{
			if (_interface) {
				_interface->RegisterTakingItemHandler(_plugin, handler);
			}
		}

		static void RegisterTakeItemHandler(TakeItemHandler handler)
		{
			if (_interface) {
				_interface->RegisterTakeItemHandler(_plugin, handler);
			}
		}

		static void RegisterSelectItemHandler(SelectItemHandler handler)
		{
			if (_interface) {
				_interface->RegisterSelectItemHandler(_plugin, handler);
			}
		}

		static void RegisterOpeningLootMenuHandler(OpeningLootMenuHandler handler)
		{
			if (_interface) {
				_interface->RegisterOpeningLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterOpenLootMenuHandler(OpenLootMenuHandler handler)
		{
			if (_interface) {
				_interface->RegisterOpenLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterCloseLootMenuHandler(CloseLootMenuHandler handler)
		{
			if (_interface) {
				_interface->RegisterCloseLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler handler)
		{
			if (_interface) {
				_interface->RegisterInvalidateLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterModifyInventoryHandler(ModifyInventoryHandler handler)
		{
			if (_interface) {
				_interface->RegisterModifyInventoryHandler(_plugin, handler);
			}
		}

		static void RegisterPopulateInfoBarHandler(PopulateInfoBarHandler handler)
		{
			if (_interface) {
				_interface->RegisterPopulateInfoBarHandler(_plugin, handler);
			}
		}

		static void RegisterPopulateButtonBarHandler(PopulateButtonBarHandler handler)
		{
			if (_interface) {
				_interface->RegisterPopulateButtonBarHandler(_plugin, handler);
			}
		}

		static void RegisterResolveStealingStateHandler(ResolveStealingStateHandler handler)
		{
			if (_interface) {
				_interface->RegisterResolveStealingStateHandler(_plugin, handler);
			}
		}

		static void RegisterResolveActionLabelHandler(ResolveActionLabelHandler handler)
		{
			if (_interface) {
				_interface->RegisterResolveActionLabelHandler(_plugin, handler);
			}
		}

		static void RegisterInputActionHandler(InputActionHandler handler)
		{
			if (_interfaceV21) {
				_interfaceV21->RegisterInputActionHandler(_plugin, handler);
			}
		}

	private:
		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		struct InterfaceV20
		{
			virtual void DisableLootMenu(const char* plugin);
			virtual void EnableLootMenu(const char* plugin);

			virtual void RegisterTakingItemHandler(const char* plugin, TakingItemHandler handler);
			virtual void RegisterTakeItemHandler(const char* plugin, TakeItemHandler handler);
			virtual void RegisterSelectItemHandler(const char* plugin, SelectItemHandler handler);
			virtual void RegisterOpeningLootMenuHandler(const char* plugin, OpeningLootMenuHandler handler);
			virtual void RegisterOpenLootMenuHandler(const char* plugin, OpenLootMenuHandler handler);
			virtual void RegisterCloseLootMenuHandler(const char* plugin, CloseLootMenuHandler handler);
			virtual void RegisterInvalidateLootMenuHandler(const char* plugin, InvalidateLootMenuHandler handler);

			virtual void RegisterModifyInventoryHandler(const char* plugin, ModifyInventoryHandler handler);
			virtual void RegisterPopulateInfoBarHandler(const char* plugin, PopulateInfoBarHandler handler);
			virtual void RegisterPopulateButtonBarHandler(const char* plugin, PopulateButtonBarHandler handler);
			virtual void RegisterResolveStealingStateHandler(const char* plugin, ResolveStealingStateHandler handler);
			virtual void RegisterResolveActionLabelHandler(const char* plugin, ResolveActionLabelHandler handler);

			virtual void ForceCurrentContainer(const char* plugin, RE::ObjectRefHandle container);
			virtual void ClearForcedContainer(const char* plugin);
			virtual void CloseLootMenu(const char* plugin);
			virtual void RefreshLootMenu(const char* plugin);
		};

		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		struct InterfaceV21 : InterfaceV20
		{
			virtual void RegisterInputActionHandler(const char* plugin, InputActionHandler handler);
		};

		static inline const char* _plugin;
		static inline InterfaceV20* _interface;
		static inline InterfaceV21* _interfaceV21;
	};
}
