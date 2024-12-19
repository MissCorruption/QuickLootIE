#pragma once

namespace QuickLoot::Util
{
	class FormUtil
	{
	public:
		FormUtil() = delete;
		~FormUtil() = delete;
		FormUtil(FormUtil const&) = delete;
		FormUtil(FormUtil const&&) = delete;
		FormUtil& operator=(FormUtil&) = delete;
		FormUtil& operator=(FormUtil&&) = delete;

		static RE::FormID ParseFormID(const std::string& identifier);

		static RE::TESForm* GetFormFromIdentifier(const std::string& identifier);

		static std::string GetIdentifierFromForm(RE::TESForm* form);
		static std::string GetIdentifierFromForm(RE::BGSRefAlias* form);

		static std::string GetModName(RE::TESForm* form);

		static bool FormExists(std::string_view pluginName, const int formId);
	};
}
