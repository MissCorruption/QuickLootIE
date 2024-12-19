#pragma once

namespace QuickLoot
{
	class MenuVisibilityManager
	{
	public:
		MenuVisibilityManager() = delete;
		~MenuVisibilityManager() = delete;
		MenuVisibilityManager(MenuVisibilityManager&&) = delete;
		MenuVisibilityManager(const MenuVisibilityManager&) = delete;
		MenuVisibilityManager& operator=(MenuVisibilityManager&&) = delete;
		MenuVisibilityManager& operator=(const MenuVisibilityManager&) = delete;

		static void InstallHooks();

		static void DisableLootMenu(const std::string& modName);
		static void EnableLootMenu(const std::string& modName);

		static void OnCameraStateChanged(RE::CameraState state);
		static void OnCombatStateChanged(RE::ACTOR_COMBAT_STATE state);
		static void OnContainerChanged(RE::FormID container);
		static void OnCrosshairRefChanged(const RE::ObjectRefHandle& ref);
		static void OnLifeStateChanged(RE::Actor& actor);
		static void OnLockChanged(RE::TESObjectREFR& container);
		static void OnMenuOpenClose(bool opening, const RE::BSFixedString& menuName);

	private:
		static constexpr bool LOG_EVENTS = false;

		static inline RE::ObjectRefHandle _focusedRef{};
		static inline RE::ObjectRefHandle _currentContainer{};
		static inline std::set<std::string> _disablingMods{};

		static RE::TESObjectREFRPtr GetContainerObject(RE::ObjectRefHandle ref);
		static bool IsValidCameraState(RE::CameraState state);
		static const char* GetMenuNameSafe(const RE::IMenu* menu);
		static const char* FindBlockingMenu();
		static bool CanOpen(const RE::TESObjectREFRPtr& container);
		static void RefreshOpenState();
		static void RefreshInventory();
	};
}
