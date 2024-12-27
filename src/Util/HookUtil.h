#pragma once

namespace QuickLoot::Util
{
	class HookUtil
	{
	public:
		HookUtil() = delete;
		~HookUtil() = delete;
		HookUtil(HookUtil const&) = delete;
		HookUtil(HookUtil const&&) = delete;
		HookUtil& operator=(HookUtil&) = delete;
		HookUtil& operator=(HookUtil&&) = delete;

		template <typename TPatch>
		static void WritePatch()
		{
			static_assert(std::is_base_of_v<Xbyak::CodeGenerator, TPatch>);

			constexpr uint64_t startOffset = TPatch::patchStart - TPatch::functionStart;
			constexpr uint64_t endOffset = TPatch::patchEnd - TPatch::functionStart;
			constexpr uint64_t size = endOffset - startOffset;
			static_assert(size >= 6);

			const REL::Relocation location{ REL::ID(TPatch::functionId), startOffset };
			REL::safe_fill(location.address(), REL::INT3, size);

			TPatch patch{};

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_branch<6>(location.address(), trampoline.allocate(patch));
		}
	};
}
