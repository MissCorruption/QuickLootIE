#pragma once

#include "Items/OldItem.h"

namespace QuickLoot::Items
{

	class OldInventoryItem final :
		public OldItem
	{
	private:
		using super = OldItem;

	public:
		OldInventoryItem() = delete;
		OldInventoryItem(const OldInventoryItem&) = delete;
		OldInventoryItem(OldInventoryItem&&) = default;

		OldInventoryItem(std::ptrdiff_t a_count, bool a_stealing, std::unique_ptr<RE::InventoryEntryData> a_item, RE::ObjectRefHandle a_container) :
			super(a_count, a_stealing, a_item.get()),
			_entry(std::move(a_item)),
			_container(a_container)
		{
			assert(_entry != nullptr);
			assert(_entry->GetObject() != nullptr);
			assert(_container);
		}

		~OldInventoryItem() = default;

		OldInventoryItem& operator=(const OldInventoryItem&) = delete;
		OldInventoryItem& operator=(OldInventoryItem&&) = default;

	protected:
		void DoTake(RE::Actor& a_dst, std::ptrdiff_t a_count) override
		{
			auto container = _container.get();
			if (!container) {
				assert(false);
				return;
			}

			const auto [leftover, queued] = GetItemsToRemove(a_count);

			const auto object = _entry->GetObject();
			a_dst.PlayPickUpSound(object, true, false);
			TryRemoveArrows3D(*container, *object);
			const auto remove =
				[&](std::int32_t a_num, RE::ExtraDataList* a_extraList, RE::ITEM_REMOVE_REASON a_reason) {
					container->RemoveItem(object, a_num, a_reason, a_extraList, std::addressof(a_dst));
					auto& player = static_cast<RE::PlayerCharacter&>(a_dst);
					player.PlayPickupEvent(object, container->GetOwner(), container.get(), RE::PlayerCharacter::EventType::kContainer);
				};

			std::function action =
				[&](std::int32_t a_num, RE::ExtraDataList* a_extraList) {
					remove(a_num, a_extraList, RE::ITEM_REMOVE_REASON::kRemove);
				};
			if (Stolen()) {
				action =
					[&](std::int32_t a_num, RE::ExtraDataList* a_extraList) {
						remove(a_num, a_extraList, RE::ITEM_REMOVE_REASON::kSteal);
						a_dst.StealAlarm(container.get(), object, a_num, static_cast<std::int32_t>(Value()), container->GetOwner(), true);
					};
			}

			for (const auto& [xList, count] : queued) {
				action(static_cast<std::int32_t>(count), xList);
			}

			if (leftover > 0) {
				action(static_cast<std::int32_t>(leftover), nullptr);
			}

			QuickLoot::Integrations::PluginServer::HandleOnTake(&a_dst, object, a_count, container.get());
		}

		void DoSelect(RE::Actor& a_dst) override
		{
			QuickLoot::Integrations::PluginServer::HandleOnSelect(&a_dst, _entry->GetObject(), Count(), _container);
		}

		void FillElementsVector(std::vector<QuickLoot::Integrations::Element>* elements) override {
			elements->push_back(QuickLoot::Integrations::Element(_entry->GetObject(), Count(), _container));
		}

	private:
		static void TryRemoveArrows3D(RE::TESObjectREFR& a_container, const RE::TESBoundObject& a_object)
		{
			if (a_object.IsAmmo() && a_container.Is(RE::FormType::ActorCharacter)) {
				auto& container = static_cast<RE::Actor&>(a_container);
				container.ClearExtraArrows();
			}
		}

		auto GetItemsToRemove(std::ptrdiff_t a_count)
			-> std::pair<std::ptrdiff_t, std::vector<std::pair<RE::ExtraDataList*, std::ptrdiff_t>>>
		{
			std::vector<std::pair<RE::ExtraDataList*, std::ptrdiff_t>> queued;
			auto toRemove = std::clamp<std::ptrdiff_t>(a_count, 0, Count());
			if (toRemove > 0 && _entry->extraLists) {
				for (auto& xList : *_entry->extraLists) {
					if (xList) {
						const auto xCount = std::clamp<std::ptrdiff_t>(xList->GetCount(), 1, toRemove);
						toRemove -= xCount;
						queued.emplace_back(xList, xCount);

						if (toRemove <= 0) {
							break;
						}
					}
				}
			}

			return { toRemove, std::move(queued) };
		}

		std::unique_ptr<RE::InventoryEntryData> _entry;
		RE::ObjectRefHandle _container;
	};
}
