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

	For each ItemInfo struct passed to GetItemInfo you are expected to initialize the object field yourself.
	Completionist will then fill out the other fields of the struct based on the provided object.
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

		static bool UseNeededItemTextColor()
		{
			return _interface && _interface->UseNeededItemTextColor();
		}

		static bool UseCollectedItemTextColor()
		{
			return _interface && _interface->UseCollectedItemTextColor();
		}

		struct IconInfo
		{
			// This needs to be provided by the callee.
			RE::TESBoundObject* object;

			// This is populated by Completionist.
			bool isCollected;
			bool isNeeded;
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

		struct ItemInfo
		{
			// This needs to be provided by the callee.
			RE::TESBoundObject* object;

			// This is populated by Completionist.
			RE::BSString decoratedName;
			uint32_t textColor;
			bool isNeeded;
			bool isCollected;
		};

		static ItemInfo GetItemInfo(RE::TESBoundObject* object)
		{
			ItemInfo info{};
			info.object = object;

			if (_interface) {
				_interface->GetItemInfo(&info, 1);
			}

			return info;
		}

		static void GetItemInfo(std::vector<ItemInfo>& info)
		{
			return GetItemInfo(info.data(), info.size());
		}

		static void GetItemInfo(ItemInfo* info, size_t infoCount)
		{
			if (_interface) {
				_interface->GetItemInfo(info, infoCount);
			}
		}

		static RE::BSString GetDecoratedItemName(RE::TESBoundObject* object)
		{
			if (_interface) {
				return _interface->GetDecoratedItemName(object);
			}

			return object->GetName();
		}

		static uint32_t GetItemTextColor(RE::TESBoundObject* object)
		{
			return _interface ? _interface->GetItemTextColor(object) : 0xffffff;
		}

		static bool IsItemNeeded(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemNeeded(object);
		}

		static bool IsItemCollected(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemCollected(object);
		}

		static bool IsItemTracked(RE::TESBoundObject* object)
		{
			return _interface && _interface->IsItemTracked(object);
		}

	private:
		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		struct InterfaceV20
		{
			virtual uint32_t GetNeededItemTextColor();
			virtual uint32_t GetCollectedItemTextColor();
			virtual bool UseNeededItemTextColor();
			virtual bool UseCollectedItemTextColor();

			virtual void GetItemInfo(ItemInfo* info, size_t count);
			virtual void GetIconInfo(IconInfo* info, size_t count);

			virtual RE::BSString GetDecoratedItemName(RE::TESBoundObject* object);
			virtual uint32_t GetItemTextColor(RE::TESBoundObject* object);
			virtual bool IsItemNeeded(RE::TESBoundObject* object);
			virtual bool IsItemCollected(RE::TESBoundObject* object);
			virtual bool IsItemTracked(RE::TESBoundObject* object);
		};

		static inline InterfaceV20* _interface;
	};
}
