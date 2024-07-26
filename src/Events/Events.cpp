#include "Events.h"

#include "LootMenuManager.h"

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <xbyak/xbyak.h>

namespace Events
{
	void CrosshairRefManager::Evaluate(RE::TESObjectREFRPtr a_ref)
	{
		auto& loot = LootMenuManager::GetSingleton();
		if (CanOpen(std::move(a_ref))) {
			loot.SetContainer(_cachedAshPile ? _cachedAshPile : _cachedRef);
		} else {
			loot.Close();
		}
	}

	void CombatManager::Close()
	{
		auto& loot = LootMenuManager::GetSingleton();
		loot.Close();
	}

	void LifeStateManager::Register()
	{
		if (REL::Module::IsAE()) {
			struct Patch :
				Xbyak::CodeGenerator
			{
				explicit Patch(std::uintptr_t a_target)
				{
					mov(rcx, rdi);  // rdi == Actor* this

					pop(r15);
					pop(r14);
					pop(rdi);
					pop(rsi);
					pop(rbp);

					mov(rax, a_target);
					jmp(rax);
				}
			};

			constexpr std::size_t begin = 0x494;
			constexpr std::size_t end = 0x49B;
			constexpr std::size_t size = end - begin;
			static_assert(size >= 6);

			REL::Relocation<std::uintptr_t> target{ REL::ID(37612), begin };  // Actor::SetLifeState
			REL::safe_fill(target.address(), REL::INT3, size);

			auto& trampoline = SKSE::GetTrampoline();
			Patch p{ reinterpret_cast<std::uintptr_t>(OnLifeStateChanged) };
			trampoline.write_branch<6>(
				target.address(),
				trampoline.allocate(p));
		} else {
			struct Patch :
				Xbyak::CodeGenerator
			{
				explicit Patch(std::uintptr_t a_target)
				{
					mov(rcx, rsi);  // rsi == Actor* this

					pop(r15);
					pop(r14);
					pop(r12);
					pop(rdi);
					pop(rsi);
					pop(rbx);
					pop(rbp);

					mov(rax, a_target);
					jmp(rax);
				}
			};

			constexpr std::size_t begin = 0x503;
			constexpr std::size_t end = 0x50D;
			constexpr std::size_t size = end - begin;
			static_assert(size >= 6);

			REL::Relocation<std::uintptr_t> target{ REL::ID(36604), begin };  // Actor::SetLifeState
			REL::safe_fill(target.address(), REL::INT3, size);

			auto& trampoline = SKSE::GetTrampoline();
			Patch p{ reinterpret_cast<std::uintptr_t>(OnLifeStateChanged) };
			trampoline.write_branch<6>(
				target.address(),
				trampoline.allocate(p));
		}

		logger::info("Registered {}"sv, typeid(LifeStateManager).name());
	}
}

