#pragma once

namespace QuickLoot::Behaviors
{
	class ActivationBlocker
	{
		static inline std::atomic_bool _blocked = false;

	public:
		ActivationBlocker() = delete;
		~ActivationBlocker() = delete;
		ActivationBlocker(ActivationBlocker&&) = delete;
		ActivationBlocker(const ActivationBlocker&) = delete;
		ActivationBlocker& operator=(ActivationBlocker&&) = delete;
		ActivationBlocker& operator=(const ActivationBlocker&) = delete;

		static void BlockActivation() noexcept { _blocked = true; }
		static void UnblockActivation() noexcept { _blocked = false; }
		[[nodiscard]] static bool IsActivationBlocked() noexcept { return _blocked; }

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

	protected:
		template <std::uint64_t RelocationID>
		class Handler
		{
			static bool GetActivateText(RE::TESBoundObject* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
			{
				if (IsActivationBlocked()) {
					return false;
				}

				return _GetActivateText(a_this, a_activator, a_dst);
			}

			static inline REL::Relocation<decltype(GetActivateText)> _GetActivateText;

		public:
			Handler() = delete;
			~Handler() = delete;

			Handler(const Handler&) = delete;
			Handler(Handler&&) = delete;
			Handler& operator=(const Handler&) = delete;
			Handler& operator=(Handler&&) = delete;

			static void Install()
			{
				REL::Relocation vtbl{ REL::ID(RelocationID) };
				_GetActivateText = vtbl.write_vfunc(0x4C, GetActivateText);
				logger::info("Installed {}", typeid(Handler).name());
			};
		};
	};
}
