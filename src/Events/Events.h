#pragma once

#ifdef GetObject
#undef GetObject
#endif

namespace Events
{
	class CrosshairRefManager :
		public RE::BSTEventSink<SKSE::CrosshairRefEvent>,
		public RE::BSTEventSink<RE::TESLockChangedEvent>
	{
	public:
		[[nodiscard]] static CrosshairRefManager* GetSingleton()
		{
			static CrosshairRefManager singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			auto crosshair = SKSE::GetCrosshairRefEventSource();
			if (crosshair) {
				crosshair->AddEventSink(GetSingleton());
				logger::info("Registered {}"sv, typeid(SKSE::CrosshairRefEvent).name());
			}

			auto scripts = RE::ScriptEventSourceHolder::GetSingleton();
			if (scripts) {
				scripts->AddEventSink<RE::TESLockChangedEvent>(GetSingleton());
				logger::info("Registered {}"sv, typeid(RE::TESLockChangedEvent).name());
			}
		}

	protected:
		friend class LifeStateManager;
		friend class LockedContainerManager;

		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>*) override
		{
			auto crosshairRef =
				a_event && a_event->crosshairRef ?
                    a_event->crosshairRef->CreateRefHandle() :
                    RE::ObjectRefHandle();
			if (_cachedRef == crosshairRef) {
				return EventResult::kContinue;
			}

			_cachedRef = crosshairRef;
			_cachedAshPile.reset();
			Evaluate(a_event->crosshairRef);

			return EventResult::kContinue;
		}

		EventResult ProcessEvent(const RE::TESLockChangedEvent* a_event, RE::BSTEventSource<RE::TESLockChangedEvent>*) override
		{
			if (a_event &&
				a_event->lockedObject &&
				a_event->lockedObject->GetHandle() == _cachedRef) {
				Evaluate(a_event->lockedObject);
			}

			return EventResult::kContinue;
		}

		void OnLifeStateChanged(RE::Actor& a_actor)
		{
			if (a_actor.GetHandle() == _cachedRef) {
				Evaluate(RE::TESObjectREFRPtr{ std::addressof(a_actor) });
			}
		}

		void OnLockChangedEvent(RE::TESObjectREFR& container)
		{
			if (container.GetHandle() == _cachedRef) {
				Evaluate(RE::TESObjectREFRPtr{ std::addressof(container) });
			}
		}

	private:
		CrosshairRefManager() = default;
		CrosshairRefManager(const CrosshairRefManager&) = delete;
		CrosshairRefManager(CrosshairRefManager&&) = delete;

		~CrosshairRefManager() = default;

		CrosshairRefManager& operator=(const CrosshairRefManager&) = delete;
		CrosshairRefManager& operator=(CrosshairRefManager&&) = delete;

		void Evaluate(RE::TESObjectREFRPtr a_ref);

		[[nodiscard]] bool CanOpen(RE::TESObjectREFRPtr a_ref)
		{
			auto obj = a_ref ? a_ref->GetObjectReference() : nullptr;
			if (!a_ref || !obj) {
				return false;
			}

			if (obj->Is(RE::FormType::Activator)) {
				_cachedAshPile = a_ref->extraList.GetAshPileRef();
				return CanOpen(_cachedAshPile.get());
			}

			//const bool disable_for_animals = Settings::DisableForAnimals();

			if (auto actor = a_ref->As<RE::Actor>(); actor) {
				//auto dobj = RE::BGSDefaultObjectManager::GetSingleton();
				//auto animal_keyword = dobj->GetObject<RE::BGSKeyword>(RE::DEFAULT_OBJECT::kKeywordAnimal);

				if (!actor->IsDead() 
					|| actor->IsSummoned())
					//|| (disable_for_animals && actor->GetRace()->HasKeyword(animal_keyword)))
				{
					return false;
				}
			}

			// Which Key? NG compatibility
			return a_ref->HasContainer() && !a_ref->IsLocked();
		}

		RE::ObjectRefHandle _cachedRef;
		RE::ObjectRefHandle _cachedAshPile;
	};

	class CombatManager :
		public RE::BSTEventSink<RE::TESCombatEvent>
	{
	public:
		static CombatManager* GetSingleton()
		{
			static CombatManager singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			auto scripts = RE::ScriptEventSourceHolder::GetSingleton();
			if (scripts) {
				scripts->AddEventSink(GetSingleton());
				logger::info("Registered {}"sv, typeid(CombatManager).name());
			}
		}

	protected:
		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*) override
		{
			if (!Settings::CloseInCombat())
				return EventResult::kContinue;

			using CombatState = RE::ACTOR_COMBAT_STATE;

			const auto isPlayerRef = [](auto&& a_ref) {
				return a_ref && a_ref->IsPlayerRef();
			};

			if (a_event && (isPlayerRef(a_event->actor) || isPlayerRef(a_event->targetActor))) {
				switch (*a_event->newState) {
				case CombatState::kCombat:
				case CombatState::kSearching:
					Close();
					break;
				default:
					break;
				}
			}

			return EventResult::kContinue;
		}

	private:
		CombatManager() = default;
		CombatManager(const CombatManager&) = delete;
		CombatManager(CombatManager&&) = delete;

		~CombatManager() = default;

		CombatManager& operator=(const CombatManager&) = delete;
		CombatManager& operator=(CombatManager&&) = delete;

		void Close();
	};

	//TESLockChangedEvent
	class LockedContainerManager :
		public RE::BSTEventSink<RE::TESLockChangedEvent>
	{
	public:
		static LockedContainerManager* GetSingleton()
		{
			static LockedContainerManager singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			auto scripts = RE::ScriptEventSourceHolder::GetSingleton();
			if (scripts) {
				scripts->AddEventSink(GetSingleton());
				logger::info("Registered {}"sv, typeid(LockedContainerManager).name());
			}
		}

	protected:
		using EventResult = RE::BSEventNotifyControl;

		EventResult ProcessEvent(const RE::TESLockChangedEvent* a_event, RE::BSTEventSource<RE::TESLockChangedEvent>*) override
		{
			if (!Settings::OpenWhenContainerUnlocked())
				return EventResult::kContinue;

			using CombatState = RE::ACTOR_COMBAT_STATE;

			CrosshairRefManager* ref_manager = CrosshairRefManager::GetSingleton();

			if (a_event && a_event->lockedObject) {
				ref_manager->OnLockChangedEvent(*a_event->lockedObject);
			}

			return EventResult::kContinue;
		}

	private:
		LockedContainerManager() = default;
		LockedContainerManager(const LockedContainerManager&) = delete;
		LockedContainerManager(LockedContainerManager&&) = delete;

		~LockedContainerManager() = default;

		LockedContainerManager& operator=(const LockedContainerManager&) = delete;
		LockedContainerManager& operator=(LockedContainerManager&&) = delete;

		void Close();
	};

	class LifeStateManager
	{
	public:
		static void Register();

	private:
		static void OnLifeStateChanged(RE::Actor* a_actor)
		{
			const auto manager = CrosshairRefManager::GetSingleton();
			manager->OnLifeStateChanged(*a_actor);
		}
	};

	inline void Register()
	{
		CrosshairRefManager::Register();
		LifeStateManager::Register();
		LockedContainerManager::Register();
		CombatManager::Register();

		logger::info("Registered all event handlers"sv);
	}
}
