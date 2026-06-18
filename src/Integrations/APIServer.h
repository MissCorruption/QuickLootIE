#pragma once

#include <shared_mutex>

#include "Items/QuickLootItemStack.h"
#include "QuickLootAPI.h"

#include <vector>

namespace QuickLoot::API
{
	class APIServer
	{
	public:
		APIServer() = delete;
		~APIServer() = delete;
		APIServer(APIServer const&) = delete;
		APIServer(APIServer const&&) = delete;
		APIServer operator=(APIServer&) = delete;
		APIServer operator=(APIServer&&) = delete;

		using InterfaceV20 = QuickLootAPI::InterfaceV20;
		using InterfaceV21 = QuickLootAPI::InterfaceV21;

		friend struct QuickLootAPI::InterfaceV20;
		friend struct QuickLootAPI::InterfaceV21;

		static InterfaceV20* GetInterfaceV20() { return &_interface; }
		static InterfaceV21* GetInterfaceV21() { return &_interface; }

		static HandleResult DispatchTakingItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static void DispatchTakeItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static void DispatchSelectItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);

		static HandleResult DispatchOpeningLootMenuEvent(RE::ObjectRefHandle container);
		static void DispatchOpenLootMenuEvent(RE::ObjectRefHandle container);
		static void DispatchCloseLootMenuEvent(RE::ObjectRefHandle container);

		static void DispatchInvalidateLootMenuEvent(RE::ObjectRefHandle container, const RE::BSTArray<ItemStack>& inventory);
		static void DispatchModifyInventoryEvent(RE::ObjectRefHandle container, RE::BSTArray<ItemStack>& inventory);

		static std::vector<RE::BSString> DispatchPopulateInfoBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static std::vector<ButtonDefinition> DispatchPopulateButtonBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static void DispatchModifyButtonBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef, RE::BSTArray<ButtonDefinition2>& buttons);

		static void DispatchModifyItemDataEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef, RE::GFxValue& data);

		static HandleResult DispatchInputActionEvent(RE::ObjectRefHandle container, QuickLootAction action);

	private:
		static inline InterfaceV21 _interface{};
		static inline std::shared_mutex _lock{};

		static inline std::vector<TakingItemHandler> _takingItemHandlers{};
		static inline std::vector<TakeItemHandler> _takeItemHandlers{};
		static inline std::vector<SelectItemHandler> _selectItemHandlers{};
		static inline std::vector<OpeningLootMenuHandler> _openingLootMenuHandlers{};
		static inline std::vector<OpenLootMenuHandler> _openLootMenuHandlers{};
		static inline std::vector<CloseLootMenuHandler> _closeLootMenuHandlers{};
		static inline std::vector<InvalidateLootMenuHandler> _invalidateLootMenuHandlers{};
		static inline std::vector<ModifyInventoryHandler> _modifyInventoryHandlers{};
		static inline std::vector<PopulateInfoBarHandler> _populateInfoBarHandlers{};
		static inline std::vector<PopulateButtonBarHandler> _populateButtonBarHandlers{};
		static inline std::vector<ModifyButtonBarHandler> _modifyButtonBarHandlers{};
		static inline std::vector<ModifyItemDataHandler> _modifyItemDataHandlers{};
		static inline std::vector<InputActionHandler> _inputActionHandlers{};

		template <typename THandler>
		static void RegisterHandler(const char* plugin, THandler handler, std::vector<THandler>& handlerList)
		{
			std::unique_lock guard(_lock);
			handlerList.push_back(handler);
			logger::info("Registered {} for {}", typeid(THandler).name(), plugin);
		}

		template <typename TEvent>
		static void DispatchEvent(const std::vector<EventHandler<TEvent>>& handlers, TEvent& e)
		{
			std::shared_lock guard(_lock);

			for (auto const& handler : handlers) {
				handler(&e);
			}
		}

		template <typename TEvent>
		static HandleResult DispatchCancelableEvent(const std::vector<EventHandler<TEvent>>& handlers, TEvent& e)
		{
			std::shared_lock guard(_lock);

			for (auto const& handler : handlers) {
				handler(&e);

				if (e.result != HandleResult::kContinue) {
					break;
				}
			}

			return e.result;
		}

		template <typename TResult, typename TEvent>
		static std::vector<TResult> DispatchResultEvent(const std::vector<EventHandler<TEvent>>& handlers, TEvent& e)
		{
			std::shared_lock guard(_lock);
			std::vector<TResult> result{};

			for (auto const& handler : handlers) {
				handler(&e);

				for (auto const& item : e.result) {
					result.push_back(std::move(item));
				}
			}

			return result;
		}
	};
}
