#include "MenuVisibilityManager.h"

#include "Config/SystemSettings.h"
#include "Config/UserSettings.h"
#include "LootMenu.h"
#include "LootMenuManager.h"
#include "Observers/CameraStateObserver.h"
#include "Observers/CombatStateObserver.h"
#include "Observers/ContainerObserver.h"
#include "Observers/CrosshairRefObserver.h"
#include "Observers/LifeStateObserver.h"
#include "Observers/LockChangedObserver.h"
#include "Observers/MenuObserver.h"

using Settings = QuickLoot::Config::UserSettings;

namespace QuickLoot
{
#pragma warning(push)
#pragma warning(disable: 4100)

	RE::TESObjectREFRPtr MenuVisibilityManager::GetContainerObject(RE::ObjectRefHandle ref)
	{
		if (auto ptr = ref.get()) {
			const auto object = ptr->GetObjectReference();

			// For enemies that leave behind an ash pile on death
			if (object->Is(RE::FormType::Activator)) {
				const auto ashPile = ptr->extraList.GetAshPileRef();
				return GetContainerObject(ashPile);
			}

			if (ptr->HasContainer()) {
				return ptr;
			}
		}

		return nullptr;
	}

	bool MenuVisibilityManager::IsValidCameraState(RE::CameraState state)
	{
		switch (state) {
		case RE::CameraState::kFirstPerson:
			return true;

		case RE::CameraState::kThirdPerson:
			return Settings::ShowInThirdPersonView();

		case RE::CameraState::kMount:
			return Settings::ShowWhenMounted();

		default:
			return false;
		}
	}

	const char* MenuVisibilityManager::GetMenuNameSafe(const RE::IMenu* menu)
	{
		const auto ui = RE::UI::GetSingleton();

		for (auto [name, entry] : ui->menuMap) {
			if (menu == entry.menu.get()) {
				return name.c_str();
			}
		}

		return "Unknown";
	}

	const char* MenuVisibilityManager::FindBlockingMenu()
	{
		const auto ui = RE::UI::GetSingleton();
		std::set<RE::IMenu*> whitelistedMenus{};

		if (const auto lootMenu = ui->GetMenu(LootMenu::MENU_NAME)) {
			whitelistedMenus.emplace(lootMenu.get());
		}

		for (auto name : Config::SystemSettings::GetMenuWhitelist()) {
			if (const auto whitelistedMenu = ui->GetMenu(name)) {
				whitelistedMenus.emplace(whitelistedMenu.get());
			}
		}

		for (auto menu : ui->menuStack) {
			if (menu->menuFlags & RE::UI_MENU_FLAGS::kAlwaysOpen)
				continue;

			// IMenu::menuName is unreliable and can even crash the game when read, so instead of
			// checking whether a menu's name is on the whitelist we need to actually compare with
			// the instances of all whitelisted menus.
			if (whitelistedMenus.contains(menu.get()))
				continue;

			return GetMenuNameSafe(menu.get());
		}

		return nullptr;
	}

	bool MenuVisibilityManager::IsContainerBlacklisted(const RE::TESObjectREFRPtr& container)
	{
		const auto& blacklist = Config::SystemSettings::GetContainerBlacklist();

		if (blacklist.contains(container->formID)) {
			return true;
		}

		if (const auto baseObj = container->GetBaseObject()) {
			return blacklist.contains(baseObj->formID);
		}

		return false;
	}

	bool MenuVisibilityManager::CanOpen(const RE::TESObjectREFRPtr& container)
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto cameraState = RE::PlayerCamera::GetSingleton()->currentState;

		if (!container) {
			return false;
		}

		if (container->IsLocked()) {
			logger::debug("LootMenu disabled because container is locked");
			return false;
		}

		if (container->IsActivationBlocked()) {
			logger::debug("LootMenu disabled because container activation is blocked");
			return false;
		}

		if (!Settings::ShowInCombat() && player->IsInCombat()) {
			logger::debug("LootMenu disabled because player is in combat");
			return false;
		}

		if (player->IsGrabbing()) {
			logger::debug("LootMenu disabled because player is grabbing something");
			return false;
		}

		if (player->HasActorDoingCommand()) {
			logger::debug("LootMenu disabled because player is commanding a follower");
			return false;
		}

		if (RE::MenuControls::GetSingleton()->InBeastForm()) {
			logger::debug("LootMenu disabled because player is in beast form");
			return false;
		}

		if (cameraState && !IsValidCameraState(cameraState->id)) {
			logger::debug("LootMenu disabled because of camera state ({})", cameraState ? static_cast<int>(cameraState->id) : -1);
			return false;
		}

		if (RE::UI::GetSingleton()->GameIsPaused()) {
			logger::debug("LootMenu disabled because the game is paused");
			return false;
		}

		if (IsContainerBlacklisted(container)) {
			logger::debug("LootMenu disabled because the container is blacklisted ({:08X})", container->formID);
			return false;
		}

		if (container->HasKeywordByEditorID("QuickLootIE_Exclude")) {
			logger::debug("LootMenu disabled because the container is marked with QuickLootIE_Exclude ({:08X})", container->formID);
			return false;
		}

		if (const char* blocking = FindBlockingMenu()) {
			logger::debug("LootMenu disabled because a blocking menu is open ({})", blocking);
			return false;
		}

		if (!_disablingMods.empty()) {
			logger::debug("LootMenu disabled by {}", *_disablingMods.begin());
			return false;
		}

		if (const auto actor = container->As<RE::Actor>()) {
			if (!actor->IsDead()) {
				logger::debug("LootMenu disabled because the actor isn't dead");
				return false;
			}

			if (actor->IsSummoned()) {
				logger::debug("LootMenu disabled because the actor is summoned");
				return false;
			}

			if (!Settings::EnableForCorpses()) {
				logger::debug("LootMenu disabled for corpses");
				return false;
			}

			if (!Settings::EnableForAnimals() && actor->HasKeywordString("ActorTypeAnimal")) {
				logger::debug("LootMenu disabled for animals");
				return false;
			}

			if (!Settings::EnableForDragons() && actor->HasKeywordString("ActorTypeDragon")) {
				logger::debug("LootMenu disabled for dragons");
				return false;
			}
		}

		return true;
	}

	void MenuVisibilityManager::RefreshOpenState()
	{
		PROFILE_SCOPE;

		const auto container = GetContainerObject(_focusedRef);
		if (CanOpen(container)) {
			_currentContainer = container->GetHandle();
			LootMenuManager::RequestShow(_currentContainer);
		} else {
			_currentContainer.reset();
			LootMenuManager::RequestHide();
		}
	}

	void MenuVisibilityManager::RefreshInventory()
	{
		LootMenuManager::RequestRefresh(RefreshFlags::kInventory);
	}

	void MenuVisibilityManager::InstallHooks()
	{
		Observers::CameraStateObserver::Install();
		Observers::CombatStateObserver::Install();
		Observers::ContainerObserver::Install();
		Observers::CrosshairRefObserver::Install();
		Observers::LifeStateObserver::Install();
		Observers::LockChangedObserver::Install();
		Observers::MenuObserver::Install();
	}

	void MenuVisibilityManager::DisableLootMenu(const std::string& modName)
	{
		_disablingMods.insert(modName);
		RefreshOpenState();
	}

	void MenuVisibilityManager::EnableLootMenu(const std::string& modName)
	{
		_disablingMods.erase(modName);
		RefreshOpenState();
	}

	void MenuVisibilityManager::OnCameraStateChanged(RE::CameraState state)
	{
		if (LOG_EVENTS) {
			logger::trace("OnCameraStateChanged: {}", std::to_underlying(state));
		}

		RefreshOpenState();
	}

	void MenuVisibilityManager::OnCombatStateChanged(RE::ACTOR_COMBAT_STATE state)
	{
		if (LOG_EVENTS) {
			logger::trace("OnCombatStateChanged: {}", std::to_underlying(state));
		}

		RefreshOpenState();
	}

	void MenuVisibilityManager::OnContainerChanged(RE::FormID container)
	{
		if (LOG_EVENTS) {
			logger::trace("OnContainerChanged: {:08X}", container);
		}

		if (_currentContainer.get() && container == _currentContainer.get()->GetFormID()) {
			RefreshInventory();
		}
	}

	void MenuVisibilityManager::OnCrosshairRefChanged(const RE::ObjectRefHandle& ref)
	{
		if (LOG_EVENTS) {
			logger::trace("OnCrosshairRefChanged: {:08X}", ref.get() ? ref.get()->GetFormID() : 0);
		}

		if (ref != _focusedRef) {
			_focusedRef = ref;
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnLifeStateChanged(RE::Actor& actor)
	{
		if (LOG_EVENTS) {
			logger::trace("OnLifeStateChanged: {:08X}", actor.GetFormID());
		}

		if (actor.GetHandle() == _focusedRef) {
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnLockChanged(RE::TESObjectREFR& container)
	{
		if (LOG_EVENTS) {
			logger::trace("OnLockChanged: {:08X}", container.GetFormID());
		}

		if (Settings::ShowWhenUnlocked() && container.GetHandle() == _focusedRef) {
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnMenuOpenClose(bool opening, const RE::BSFixedString& menuName)
	{
		if (LOG_EVENTS) {
			logger::trace("OnMenuOpenClose: {} {}", opening ? "Open" : "Close", menuName);
		}

		// Always ignore events related to the loot menu to avoid feedback loops
		if (menuName == LootMenu::MENU_NAME) {
			return;
		}

		if (!opening && menuName == RE::LockpickingMenu::MENU_NAME && !Settings::ShowWhenUnlocked()) {
			// Without this the activation prompt will continue to show the container as locked
			RE::PlayerCharacter::GetSingleton()->UpdateCrosshairs();

			// Don't refresh open state when ShowWhenUnlocked is false
			return;
		}

		if (!opening && menuName == RE::JournalMenu::MENU_NAME) {
			Settings::Update();
		}

		RefreshOpenState();
	}

#pragma warning(pop)
}
