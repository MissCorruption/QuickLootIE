#pragma once

namespace QuickLoot
{
	// TODO move this out of the hooks namespace
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

		static void OnCameraStateChanged(RE::CameraState state);
		static void OnCombatStateChanged(RE::ACTOR_COMBAT_STATE state);
		static void OnContainerChanged(RE::FormID container);
		static void OnCrosshairRefChanged(const RE::ObjectRefHandle& ref);
		static void OnLifeStateChanged(RE::Actor& actor);
		static void OnLockChanged(RE::TESObjectREFR& container);
		static void OnMenuOpenClose(bool opening, const char* menuName);

	private:
		static inline RE::CameraState _cameraState = RE::CameraState::kFirstPerson;
		static inline bool _isInCombat = false;

		static inline RE::ObjectRefHandle _focusedRef{};
		static inline RE::ObjectRefHandle _displayedContainer{};

		static RE::TESObjectREFRPtr GetContainerObject(RE::ObjectRefHandle ref);
		static bool CanOpen(const RE::TESObjectREFRPtr& container);
		static void RefreshOpenState();
		static void RefreshInventory();
	};
}
