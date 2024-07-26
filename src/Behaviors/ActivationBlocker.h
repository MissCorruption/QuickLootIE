#pragma once

namespace QuickLoot::Behaviors
{
	class ActivationBlocker
	{
	public:
		static ActivationBlocker& GetSingleton()
		{
			static ActivationBlocker singleton;
			return singleton;
		}

		static void Install()
		{
			if (REL::Module::IsAE()) {
				Handler<195816>::Install();  // TESNPC
				Handler<189485>::Install();  // TESObjectACTI
				Handler<189633>::Install();  // TESObjectCONT
			} else {
				Handler<241857>::Install();  // TESNPC
				Handler<234000>::Install();  // TESObjectACTI
				Handler<234148>::Install();  // TESObjectCONT
			}
			logger::info("Installed {}"sv, typeid(ActivationBlocker).name());
		}

		void Enable() noexcept { _enabled = true; }
		void Disable() noexcept { _enabled = false; }

	protected:
		template <std::uint64_t ID>
		class Handler
		{
		public:
			static void Install()
			{
				REL::Relocation<std::uintptr_t> vtbl{ REL::ID(ID) };
				_func = vtbl.write_vfunc(0x4C, GetActivateText);
				logger::info("Installed {}"sv, typeid(Handler).name());
			}

		private:
			Handler() = delete;
			Handler(const Handler&) = delete;
			Handler(Handler&&) = delete;

			~Handler() = delete;

			Handler& operator=(const Handler&) = delete;
			Handler& operator=(Handler&&) = delete;

			static bool GetActivateText(RE::TESBoundObject* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
			{
				const auto& proxy = ActivationBlocker::GetSingleton();
				if (proxy.Enabled()) {
					return false;
				} else {
					return _func(a_this, a_activator, a_dst);
				}
			}

			static inline REL::Relocation<decltype(GetActivateText)> _func;
		};

		[[nodiscard]] bool Enabled() const noexcept { return _enabled; }
		[[nodiscard]] bool Disabled() const noexcept { return !_enabled; }

	private:
		constexpr ActivationBlocker() noexcept = default;
		ActivationBlocker(const ActivationBlocker&) = delete;
		ActivationBlocker(ActivationBlocker&&) = delete;

		~ActivationBlocker() = default;

		ActivationBlocker& operator=(const ActivationBlocker&) = delete;
		ActivationBlocker& operator=(ActivationBlocker&&) = delete;

		std::atomic_bool _enabled{ false };
	};
}
