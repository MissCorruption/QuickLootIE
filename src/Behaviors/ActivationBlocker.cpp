#include "ActivationBlocker.h"

namespace QuickLoot::Behaviors
{
	template <std::uint64_t VTableRelocationID>
	class GetActivateTextHook
	{
		static bool GetActivateText(RE::TESBoundObject* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
		{
			if (ActivationBlocker::IsActivationBlocked()) {
				return false;
			}

			return _GetActivateText(a_this, a_activator, a_dst);
		}

		static inline REL::Relocation<decltype(GetActivateText)> _GetActivateText;

	public:
		GetActivateTextHook() = delete;
		~GetActivateTextHook() = delete;
		GetActivateTextHook(const GetActivateTextHook&) = delete;
		GetActivateTextHook(GetActivateTextHook&&) = delete;
		GetActivateTextHook& operator=(const GetActivateTextHook&) = delete;
		GetActivateTextHook& operator=(GetActivateTextHook&&) = delete;

		static void Install()
		{
			REL::Relocation vTable{ REL::ID(VTableRelocationID) };
			_GetActivateText = vTable.write_vfunc(0x4C, GetActivateText);

			logger::info("Installed {}", typeid(GetActivateTextHook).name());
		};
	};

	void ActivationBlocker::Install()
	{
		if (REL::Module::IsAE()) {
			GetActivateTextHook<195816>::Install();  // TESNPC
			GetActivateTextHook<189485>::Install();  // TESObjectACTI
			GetActivateTextHook<189633>::Install();  // TESObjectCONT
		} else {
			GetActivateTextHook<241857>::Install();  // TESNPC
			GetActivateTextHook<234000>::Install();  // TESObjectACTI
			GetActivateTextHook<234148>::Install();  // TESObjectCONT
		}

		logger::info("Installed {}", typeid(ActivationBlocker).name());
	}
}
