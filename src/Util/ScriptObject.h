#pragma once

using ScriptObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
using ScriptArrayPtr = RE::BSTSmartPointer<RE::BSScript::Array>;
using ScriptCallbackPtr = RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor>;

namespace QuickLoot::Util
{
	class ScriptObject
	{
	public:
		ScriptObject() = default;
		explicit ScriptObject(ScriptObjectPtr object) :
			_object(std::move(object)) {}

		static ScriptObject FromForm(RE::TESForm* form, const std::string& scriptName);
		static ScriptObject FromForm(const RE::BGSRefAlias* form, const std::string& scriptName);

		bool IsValid() const;
		bool IsType(const char* scriptName) const;

		RE::BSScript::Variable* GetVariable(std::string_view variableName) const;
		RE::BSScript::Variable* GetProperty(std::string_view propertyName) const;

		bool GetBool(std::string_view variableName) const;
		void SetBool(std::string_view variableName, bool value) const;

		std::int32_t GetInt(std::string_view variableName) const;
		void SetInt(std::string_view variableName, std::int32_t value) const;

		float GetFloat(std::string_view variableName) const;
		void SetFloat(std::string_view variableName, float value) const;

		std::string GetString(std::string_view variableName) const;
		void SetString(std::string_view variableName, std::string_view value) const;

		ScriptArrayPtr GetArray(std::string_view variableName) const;

		void RegisterForModEvent(RE::BSFixedString eventName, RE::BSFixedString callbackName);
		void UnregisterForModEvent(RE::BSFixedString eventName);

	private:
		ScriptObjectPtr _object{};
	};
}
