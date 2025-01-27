#include "LifeStateObserver.h"

#include "MenuVisibilityManager.h"
#include "Util/HookUtil.h"

namespace QuickLoot::Observers
{
	struct PatchSE : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 36604;
		static constexpr uint64_t functionStart = 0x5EDEF0;
		static constexpr uint64_t patchStart = 0x5EE3F3;
		static constexpr uint64_t patchEnd = 0x5EE3FE;

		explicit PatchSE()
		{
			mov(rcx, rsi);  // rsi == Actor* this

			pop(r15);
			pop(r14);
			pop(r12);
			pop(rdi);
			pop(rsi);
			pop(rbx);
			pop(rbp);

			mov(rax, reinterpret_cast<uintptr_t>(MenuVisibilityManager::OnLifeStateChanged));
			jmp(rax);
		}
	};

	struct PatchAE : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 37612;
		static constexpr uint64_t functionStart = 0x680740;
		static constexpr uint64_t patchStart = 0x680BD4;
		static constexpr uint64_t patchEnd = 0x680BDC;

		explicit PatchAE()
		{
			mov(rcx, rdi);  // rdi == Actor* this

			pop(r15);
			pop(r14);
			pop(rdi);
			pop(rsi);
			pop(rbp);

			mov(rax, reinterpret_cast<uintptr_t>(MenuVisibilityManager::OnLifeStateChanged));
			jmp(rax);
		}
	};

	struct PatchVR : Xbyak::CodeGenerator
	{
		static constexpr uint64_t functionId = 36604;
		static constexpr uint64_t functionStart = 0x5F6600;
		static constexpr uint64_t patchStart = 0x5F6B03;
		static constexpr uint64_t patchEnd = 0x5F6B0E;

		explicit PatchVR()
		{
			mov(rcx, rsi);  // rdi == Actor* this

			pop(r15);
			pop(r14);
			pop(r12);
			pop(rdi);
			pop(rsi);
			pop(rbx);
			pop(rbp);

			mov(rax, reinterpret_cast<uintptr_t>(MenuVisibilityManager::OnLifeStateChanged));
			jmp(rax);
		}
	};

	void LifeStateObserver::Install()
	{
		// Add OnLifeStateChanged as a tail call in Actor::SetLifeState

		switch (REL::Module::GetRuntime()) {
		case REL::Module::Runtime::AE:
			Util::HookUtil::WritePatch<PatchAE>();
			break;

		case REL::Module::Runtime::SE:
			Util::HookUtil::WritePatch<PatchSE>();
			break;

		case REL::Module::Runtime::VR:
			Util::HookUtil::WritePatch<PatchVR>();
			break;

		default:
			logger::error("Invalid runtime");
			break;
		}

		logger::info("Installed");
	}
}
