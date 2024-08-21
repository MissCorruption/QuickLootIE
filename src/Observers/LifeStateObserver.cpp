#include "LifeStateObserver.h"

#include "MenuVisibilityManager.h"

namespace QuickLoot::Observers
{
	void OnLifeStateChanged(RE::Actor& actor)
	{
		MenuVisibilityManager::OnLifeStateChanged(actor);
	}

	void InstallSE()
	{
		struct Patch : Xbyak::CodeGenerator
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

		const REL::Relocation target{ REL::ID(36604), begin };  // Actor::SetLifeState
		REL::safe_fill(target.address(), REL::INT3, size);

		Patch patch{ reinterpret_cast<std::uintptr_t>(OnLifeStateChanged) };

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(target.address(), trampoline.allocate(patch));
	}

	void InstallAE()
	{
		struct Patch : Xbyak::CodeGenerator
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

		const REL::Relocation target{ REL::ID(37612), begin };  // Actor::SetLifeState
		REL::safe_fill(target.address(), REL::INT3, size);

		Patch patch{ reinterpret_cast<std::uintptr_t>(OnLifeStateChanged) };

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(target.address(), trampoline.allocate(patch));
	}

	void LifeStateObserver::Install()
	{
		// Add OnLifeStateChanged as a tail call in Actor::SetLifeState

		if (REL::Module::IsAE()) {
			InstallAE();
		} else {
			InstallSE();
		}

		logger::info("Installed");
	}
}
