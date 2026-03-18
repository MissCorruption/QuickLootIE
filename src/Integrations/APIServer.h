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

			virtual void ForceCurrentContainer(const char* plugin, RE::ObjectRefHandle container);
			virtual void ClearForcedContainer(const char* plugin);
			virtual void CloseLootMenu(const char* plugin);
			virtual void RefreshLootMenu(const char* plugin);
		};

		static InterfaceV20* GetInterfaceV20() { return &_interface; }

		static HandleResult DispatchTakingItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static void DispatchTakeItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static void DispatchSelectItemEvent(RE::Actor* actor, RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);

		static HandleResult DispatchOpeningLootMenuEvent(RE::ObjectRefHandle container);
		static void DispatchOpenLootMenuEvent(RE::ObjectRefHandle container);
		static void DispatchCloseLootMenuEvent(RE::ObjectRefHandle container);

		static void DispatchInvalidateLootMenuEvent(RE::ObjectRefHandle container, const std::vector<std::unique_ptr<Items::QuickLootItemStack>>& inventory);

		static void DispatchModifyInventoryEvent(RE::ObjectRefHandle container, std::vector<std::unique_ptr<Items::QuickLootItemStack>>& inventory);
		static std::vector<RE::BSString> DispatchPopulateInfoBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);
		static std::vector<ButtonDefinition> DispatchPopulateButtonBarEvent(RE::ObjectRefHandle container, RE::InventoryEntryData* entry, RE::ObjectRefHandle dropRef);

	private:
		static inline InterfaceV20 _interface{};
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
