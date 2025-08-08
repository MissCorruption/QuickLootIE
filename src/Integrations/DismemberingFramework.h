#pragma once

namespace QuickLoot::Integrations
{
	class DismemberingFramework
	{
	public:
		DismemberingFramework() = delete;
		~DismemberingFramework() = delete;
		DismemberingFramework(DismemberingFramework const&) = delete;
		DismemberingFramework(DismemberingFramework const&&) = delete;
		DismemberingFramework operator=(DismemberingFramework&) = delete;
		DismemberingFramework operator=(DismemberingFramework&&) = delete;

		static bool Init()
		{
			// Nothing to do for now

			return IsReady();
		}

		static bool IsReady()
		{
			return GetModuleHandleA("DismemberingFramework") != nullptr;
		}

		static RE::TESObjectREFR* GetLimbOwner(const RE::TESObjectREFR* limb)
		{
			if (!IsReady()) {
				return nullptr;
			}

			if (!limb->GetObjectReference()->HasKeywordByEditorID("DF_Limb_Keyword")) {
				return nullptr;
			}

			const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			const auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
			const auto handle = policy ? policy->GetHandleForObject(limb->GetFormType(), limb) : 0;
			if (!handle) {
				return nullptr;
			}

			const auto it = vm->attachedScripts.find(handle);
			if (it == vm->attachedScripts.end()) {
				return nullptr;
			}

#undef GetObject

			const auto& scripts = it->second;
			for (const auto& script : scripts) {
				if (!IsScriptOfType(*script, "DFLimbRefBaseScript")) {
					continue;
				}

				const auto variable = script->GetVariable("ReferenceActor");
				if (!variable) {
					continue;
				}

				const auto object = variable->GetObject();
				if (!object || !IsScriptOfType(*object, "ObjectReference")) {
					continue;
				}

				const auto vmActorHandle = object->GetHandle();
				const auto actor = policy->GetObjectForHandle(RE::FormType::ActorCharacter, vmActorHandle);

				return skyrim_cast<RE::TESObjectREFR*>(actor);
			}

			return nullptr;
		}

	private:
		static bool IsScriptOfType(RE::BSScript::Object& script, const char* typeName)
		{
			auto type = script.GetTypeInfo();

			while (type != nullptr) {
				if (_stricmp(type->GetName(), typeName) == 0) {
					return true;
				}

				type = type->GetParent();
			}

			return false;
		}
	};
}
