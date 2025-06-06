#pragma once

/*
	Header File for Completionist integration.
	Copy this file into your SKSE plugin's source tree.

	Functionality is provided via static methods in the Completionist class.
	Before calling any of those, invoke Completionist::Init.

	For performance critical paths like retrieving item information for an entire inventory of items, it
	is recommended to use the GetItemInfo function rather than querying the properties (needed, collected,
	color, name) separately via their respective methods. GetItemInfo is optimized to minimize the number of
	formId lookups. It also allows querying information for multiple items at the same time.

	For each ItemInfo struct passed to GetItemInfo you are expected to initialize the entry field yourself.
	Completionist will then fill out the other fields of the struct based on the provided item.
*/

namespace QuickLoot::Integrations
{
	class Completionist
	{
	public:
		Completionist() = delete;
		~Completionist() = delete;
		Completionist(Completionist const&) = delete;
		Completionist(Completionist const&&) = delete;
		Completionist operator=(Completionist&) = delete;
		Completionist operator=(Completionist&&) = delete;

		static constexpr const char* SERVER_PLUGIN_NAME = "Completionist";

		// Call this before any other API function.
		static bool Init()
		{
			using GetInterfaceProc = InterfaceV20* (*)();

			const auto dllHandle = GetModuleHandleA(SERVER_PLUGIN_NAME);
			const auto getInterfaceProc = reinterpret_cast<GetInterfaceProc>(GetProcAddress(dllHandle, "GetCompletionistInterfaceV20"));

			if (getInterfaceProc) {
				_interface = getInterfaceProc();
			}

			return IsReady();
		}

		static bool IsReady()
		{
			return _interface;
		}

		static uint32_t GetNeededItemTextColor()
		{
			return _interface ? _interface->GetNeededItemTextColor() : 0xffffff;
		}

		static uint32_t GetCollectedItemTextColor()
		{
			return _interface ? _interface->GetCollectedItemTextColor() : 0xffffff;
		}

		static uint32_t GetOccupiedItemTextColor()
		{
			return _interface ? _interface->GetOccupiedItemTextColor() : 0xffffff;
		}

		static uint32_t GetDisplayableItemTextColor()
		{
			return _interface ? _interface->GetDisplayableItemTextColor() : 0xffffff;
		}

		static uint32_t GetDisplayedItemTextColor()
		{
			return _interface ? _interface->GetDisplayedItemTextColor() : 0xffffff;
		}

		static bool UseNeededItemTextColor()
		{
			return _interface && _interface->UseNeededItemTextColor();
		}

		static bool UseCollectedItemTextColor()
		{
			return _interface && _interface->UseCollectedItemTextColor();
		}

		static bool UseDisplayableItemTextColor()
		{
			return _interface && _interface->UseDisplayableItemTextColor();
		}

		static bool UseDisplayedItemTextColor()
		{
			return _interface && _interface->UseDisplayedItemTextColor();
		}

		static bool UseOccupiedItemTextColor()
		{
			return _interface && _interface->UseOccupiedItemTextColor();
		}

		// Icon info

		struct IconInfo
		{
			// This needs to be provided by the callee.
			RE::TESBoundObject* object;

			// This is populated by Completionist.
			bool isCollected;
			bool isNeeded;
			bool isDisplayable;
			bool isDisplayed;
			bool isOccupied;

			std::string GetRequiredIconName() const {
				if (isDisplayed)    return "compDisplayed";
				if (isOccupied)     return "compOccupied";
				if (isDisplayable)  return "compDisplayable";
				if (isCollected)    return "compFound";
				if (isNeeded)       return "compNew";
				return "";
			}
		};

		static IconInfo GetIconInfo(RE::TESBoundObject* object)
		{
			IconInfo info{};
			info.object = object;

			if (_interface) {
				_interface->GetIconInfo(&info, 1);
			}

			return info;
		}

		static void GetIconInfo(IconInfo* info, size_t count)
		{
			if (_interface) {
				_interface->GetIconInfo(info, count);
			}
		}

		static void GetIconInfo(std::vector<IconInfo>& info)
		{
			return GetIconInfo(info.data(), info.size());
		}

		// Item info

		struct ItemInfo
		{
			// This needs to be provided by the callee.
			RE::InventoryEntryData* entry;

			// This is populated by Completionist.
			RE::BSString decoratedName;
			uint32_t textColor;
			bool isNeeded;
			bool isCollected;
			bool isDisplayable;
			bool isDisplayed;
			bool isOccupied;

			std::string GetRequiredIconName() const {
				if (isDisplayed)    return "compDisplayed";
				if (isOccupied)     return "compOccupied";
				if (isDisplayable)  return "compDisplayable";
				if (isCollected)    return "compFound";
				if (isNeeded)       return "compNew";
				return "";
			}
		};

		static ItemInfo GetItemInfo(RE::TESBoundObject* object)
		{
			RE::InventoryEntryData entry{ object, 1 };

			return GetItemInfo(&entry);
		}

		static ItemInfo GetItemInfo(RE::InventoryEntryData* entry)
		{
			ItemInfo info{};
			info.entry = entry;

			if (_interface) {
				_interface->GetItemInfo(&info, 1);
			}

			return info;
		}

		static void GetItemInfo(ItemInfo* info, size_t count)
		{
			if (_interface) {
				_interface->GetItemInfo(info, count);
			}
		}

		static void GetItemInfo(std::vector<ItemInfo>& info)
		{
			return GetItemInfo(info.data(), info.size());
		}

		static RE::BSString GetDecoratedItemName(RE::TESBoundObject* object)
		{
			RE::InventoryEntryData entry{ object, 1 };

			return GetDecoratedItemName(&entry);
		}

		static RE::BSString GetDecoratedItemName(RE::InventoryEntryData* entry)
		{
			if (_interface) {
				return _interface->GetDecoratedItemName(entry);
			}

			return entry->GetDisplayName();
		}

		static uint32_t GetItemTextColor(RE::TESBoundObject* object)
		{
			return _interface ? _interface->GetItemTextColor(object) : 0xffffff;
		}

		/// Checks if the item has been collected / picked up by the player.
		static bool IsItemNeeded(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemNeeded(object);
		}

		/// Checks if the item has been collected / picked up by the player.
		static bool IsItemCollected(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemCollected(object);
		}

		/// Checks if the item is tracked by Completionist.
		static bool IsItemTracked(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemTracked(object);
		}

		/// Checks if the item is flagged as displayable in the Museum.
		static bool IsItemDisplayable(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemDisplayable(object);
		}

		/// Checks if the item is displayed in the Museum.
		/// If 'checkKnownVariations' is true, then it will also check if any of this item's variations are displayed.
		static bool IsItemDisplayed(RE::TESBoundObject* object, bool& matchedByVariation)
		{
			return _interface && _interface->IsItemDisplayed(object, matchedByVariation);
		}

	private:
		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		struct InterfaceV20
		{
			virtual uint32_t GetNeededItemTextColor();
			virtual uint32_t GetCollectedItemTextColor();
			virtual uint32_t GetOccupiedItemTextColor();
			virtual uint32_t GetDisplayableItemTextColor();
			virtual uint32_t GetDisplayedItemTextColor();

			virtual bool UseNeededItemTextColor();
			virtual bool UseCollectedItemTextColor();
			virtual bool UseDisplayableItemTextColor();
			virtual bool UseDisplayedItemTextColor();
			virtual bool UseOccupiedItemTextColor();

			virtual void GetItemInfo(ItemInfo* info, size_t count);
			virtual void GetIconInfo(IconInfo* info, size_t count);

			virtual RE::BSString GetDecoratedItemName(RE::InventoryEntryData* entry);
			virtual uint32_t GetItemTextColor(RE::TESBoundObject* object);
			virtual bool IsItemNeeded(RE::TESBoundObject* object);
			virtual bool IsItemCollected(RE::TESBoundObject* object);
			virtual bool IsItemTracked(RE::TESBoundObject* object);
			virtual bool IsItemDisplayable(RE::TESBoundObject* object);
			virtual bool IsItemDisplayed(RE::TESBoundObject* object, bool& matchedByVariation);
		};

		static inline InterfaceV20* _interface;
	};
}
