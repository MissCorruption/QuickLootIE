#include "MenuVisibilityManager.h"

#include "LootMenuManager.h"
#include "Observers/CameraStateObserver.h"
#include "Observers/CombatStateObserver.h"
#include "Observers/ContainerObserver.h"
#include "Observers/CrosshairRefObserver.h"
#include "Observers/LifeStateObserver.h"
#include "Observers/LockChangedObserver.h"
#include "Observers/MenuObserver.h"
#include "Config/Papyrus.h"

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

		if (Settings::CloseInCombat() && player->IsInCombat()) {
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
			logger::debug("LootMenu disabled because of camera state");
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

			if (Settings::DisableForAnimals() && actor->HasKeywordString("ActorTypeAnimal")) {
				logger::debug("LootMenu disabled for animals");
				return false;
			}

			if (Settings::DisableForMonsters() && actor->HasKeywordString("ActorTypeDragon")) {
				logger::debug("LootMenu disabled for dragons");
				return false;
			}
		}

		return true;
	}

	void MenuVisibilityManager::RefreshOpenState()
	{
		const auto container = GetContainerObject(_focusedRef);
		if (CanOpen(container)) {
			_currentContainer = container->GetHandle();
			LootMenuManager::SetContainer(_currentContainer);
		} else {
			_currentContainer.reset();
			LootMenuManager::Close();
		}
	}

	void MenuVisibilityManager::RefreshInventory()
	{
		LootMenuManager::RefreshInventory();
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

	void MenuVisibilityManager::OnCameraStateChanged(RE::CameraState state)
	{
		logger::trace("OnCameraStateChanged: {}", std::to_underlying(state));

		RefreshOpenState();
	}

	void MenuVisibilityManager::OnCombatStateChanged(RE::ACTOR_COMBAT_STATE state)
	{
		logger::trace("OnCombatStateChanged: {}", std::to_underlying(state));

		RefreshOpenState();
	}

	void MenuVisibilityManager::OnContainerChanged(RE::FormID container)
	{
		logger::trace("OnContainerChanged: {:08X}", container);

		if (_currentContainer.get() && container == _currentContainer.get()->GetFormID()) {
			RefreshInventory();
		}
	}

	void MenuVisibilityManager::OnCrosshairRefChanged(const RE::ObjectRefHandle& ref)
	{
		logger::trace("OnCrosshairRefChanged: {:08X}", ref.get() ? ref.get()->GetFormID() : 0);

		if (ref != _focusedRef) {
			_focusedRef = ref;
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnLifeStateChanged(RE::Actor& actor)
	{
		logger::trace("OnLifeStateChanged: {:08X}", actor.GetFormID());

		if (actor.GetHandle() == _focusedRef) {
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnLockChanged(RE::TESObjectREFR& container)
	{
		logger::trace("OnLockChanged: {:08X}", container.GetFormID());

		if (Settings::OpenWhenContainerUnlocked() && container.GetHandle() == _focusedRef) {
			RefreshOpenState();
		}
	}

	void MenuVisibilityManager::OnMenuOpenClose(bool opening, const RE::BSFixedString& menuName)
	{
		logger::trace("OnMenuOpenClose: {} {}", opening ? "Open" : "Close", menuName);

		// TODO move Papyrus::UpdateVariables() here to make sure it is executed before the refresh

		// TODO hide this behavior behind a setting
		RefreshOpenState();
	}

#pragma warning(pop)
}
