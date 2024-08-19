#include "ScriptObject.h"

#include "FormUtil.h"

namespace QuickLoot::Util
{
	ScriptObject ScriptObject::FromForm(RE::TESForm* form, const std::string& scriptName)
	{
		if (!form) {
			logger::error("Cannot retrieve script object from a None form.");
			return {};
		}

		const auto papyrusVM = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!papyrusVM) {
			logger::error("Unable to obtain Papyrus VM.");
		}

		const auto typeID = static_cast<RE::VMTypeID>(form->GetFormType());
		const auto handle = papyrusVM->handlePolicy->GetHandleForObject(typeID, form);

		ScriptObjectPtr object;
		papyrusVM->FindBoundObject(handle, scriptName.c_str(), object);

		if (!object) {
			std::string formIdentifier = FormUtil::GetIdentifierFromForm(form);
			logger::error("Script {} is not attached to form [{}]", scriptName, formIdentifier);
		}

		return ScriptObject(object);
	}

	ScriptObject ScriptObject::FromForm(const RE::BGSRefAlias* form, const std::string& scriptName)
	{
		if (!form) {
			return {};
		}

		const auto papyrusVM = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!papyrusVM) {
			return {};
		}

		const auto handle = papyrusVM->handlePolicy->GetHandleForObject(RE::BGSRefAlias::VMTYPEID, form);
		if (!handle) {
			return {};
		}

		ScriptObjectPtr object{};
		papyrusVM->FindBoundObject(handle, scriptName.c_str(), object);
		return ScriptObject(object);
	}

	bool ScriptObject::IsValid() const
	{
		return _object != nullptr;
	}

	RE::BSScript::Variable* ScriptObject::GetVariable(std::string_view variableName) const
	{
		constexpr auto INVALID = static_cast<uint32_t>(-1);

		uint32_t idx = INVALID;
		uint32_t offset = 0;

		for (auto cls = _object->type.get(); cls; cls = cls->GetParent()) {
			const auto vars = cls->GetVariableIter();
			if (idx == INVALID) {
				if (vars) {
					for (uint32_t i = 0; i < cls->GetNumVariables(); i++) {
						const auto& var = vars[i];
						if (var.name == variableName) {
							idx = i;
							break;
						}
					}
				}
			} else {
				offset += cls->GetNumVariables();
			}
		}

		if (idx == INVALID) {
			logger::debug(
				"Variable {} does not exist on script {}"sv,
				variableName,
				_object->GetTypeInfo()->GetName());

			return nullptr;
		}

		return std::addressof(_object->variables[offset + idx]);
	}

	RE::BSScript::Variable* ScriptObject::GetProperty(std::string_view propertyName) const
	{
		if (!_object || propertyName.empty()) {
			return nullptr;
		}

		if (auto* var = _object->GetProperty(propertyName)) {
			return var;
		}

		logger::debug(
			"Variable {} does not exist on script {}"sv,
			propertyName,
			_object->GetTypeInfo()->GetName());

		return nullptr;
	}

	bool ScriptObject::IsType(const char* scriptName) const
	{
		for (auto cls = _object ? _object->type.get() : nullptr; cls; cls = cls->GetParent()) {
			if (_stricmp(cls->GetName(), scriptName) == 0) {
				return true;
			}
		}

		return false;
	}

	bool ScriptObject::GetBool(std::string_view variableName) const
	{
		const auto variable = GetVariable(variableName);
		return variable ? variable->GetBool() : false;
	}

	void ScriptObject::SetBool(std::string_view variableName, bool value) const
	{
		if (const auto variable = GetVariable(variableName))
			variable->SetBool(value);
	}

	std::int32_t ScriptObject::GetInt(std::string_view variableName) const
	{
		const auto variable = GetVariable(variableName);
		return variable ? variable->GetSInt() : 0;
	}

	void ScriptObject::SetInt(std::string_view variableName, std::int32_t value) const
	{
		if (const auto variable = GetVariable(variableName))
			variable->SetSInt(value);
	}

	float ScriptObject::GetFloat(std::string_view variableName) const
	{
		const auto variable = GetVariable(variableName);
		return variable ? variable->GetFloat() : 0.0f;
	}

	void ScriptObject::SetFloat(std::string_view variableName, float value) const
	{
		if (const auto variable = GetVariable(variableName))
			variable->SetFloat(value);
	}

	std::string ScriptObject::GetString(std::string_view variableName) const
	{
		const auto variable = GetVariable(variableName);
		return variable ? std::string{ variable->GetString() } : "";
	}

	void ScriptObject::SetString(std::string_view variableName, std::string_view value) const
	{
		if (const auto variable = GetVariable(variableName))
			variable->SetString(value);
	}

	ScriptArrayPtr ScriptObject::GetArray(std::string_view variableName) const
	{
		const auto variable = GetVariable(variableName);
		return variable ? variable->GetArray() : nullptr;
	}

	void ScriptObject::RegisterForModEvent(RE::BSFixedString eventName, RE::BSFixedString callbackName)
	{
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		const auto vm = skyrimVM ? skyrimVM->impl : nullptr;

		if (vm) {
			const auto args = RE::MakeFunctionArguments(std::move(eventName), std::move(callbackName));
			ScriptCallbackPtr nullCallback{};
			vm->DispatchMethodCall(_object, "RegisterForModEvent"sv, args, nullCallback);
			delete args;
		}
	}

	void ScriptObject::UnregisterForModEvent(RE::BSFixedString eventName)
	{
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		const auto vm = skyrimVM ? skyrimVM->impl : nullptr;

		if (vm) {
			const auto args = RE::MakeFunctionArguments(std::move(eventName));
			ScriptCallbackPtr nullCallback{};
			vm->DispatchMethodCall(_object, "UnregisterForModEvent"sv, args, nullCallback);
			delete args;
		}
	}
}
